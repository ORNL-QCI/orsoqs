#ifdef SWIG_PYTHON
#	include "Python.h"
#endif

#include "socket.h"

int _or_socket_proc_rdtimeo = 500;
int _or_socket_proc_wttimeo = 500;

struct or_socket_t* or_connect(struct or_client_t *c,
		const char* const loc,
		const enum or_contype_t ct) {
	struct or_socket_t* ret = (struct or_socket_t*)malloc(sizeof(struct or_socket_t));
	int rc;
	
	uint64_t len = strlen(loc)+1;
	ret->endpoint = malloc(len);
	memcpy(ret->endpoint, loc, len);
	
	ret->contype = ct;
	
	switch(ret->contype) {
	 case CON_PUSH:
	 case CON_REQUEST:
		ret->sproc = NULL;
		ret->sock = zmq_socket(c->zcontext, ZMQ_PAIR);
		if(ret->sock == NULL) {
			printf("Error creating socket: %s\n", zmq_strerror(errno));
			abort();
		}
		rc = zmq_connect(ret->sock, ret->endpoint);
		if(rc == -1) {
			printf("Error connecting socket: %s\n", zmq_strerror(errno));
			abort();
		}
		
		break;
	
	 case CON_WAIT:
	 case CON_REPLY:
		// This code is common to both
		ret->sproc = (struct _or_socket_proc_t*)malloc(sizeof(struct _or_socket_proc_t));
		ret->sproc->doExit = false;
		ret->sock = zmq_socket(c->zcontext, ZMQ_PAIR);
		if(ret->sock == NULL) {
			printf("Error creating socket: %s\n", zmq_strerror(errno));
			abort();
		}
		zmq_setsockopt(ret->sock,
				ZMQ_RCVTIMEO,
				&_or_socket_proc_rdtimeo,
				sizeof(_or_socket_proc_rdtimeo));
		zmq_setsockopt(ret->sock,
				ZMQ_SNDTIMEO,
				&_or_socket_proc_wttimeo,
				sizeof(_or_socket_proc_wttimeo));
		rc = zmq_connect(ret->sock, ret->endpoint);
		if(rc == -1) {
			printf("Error connecting socket: %s\n", zmq_strerror(errno));
			abort();
		}
		
		#ifdef SWIG_PYTHON
		// This is *REALLY* important as it allows us to call PyGILState_Ensure() and
		// PyGILState_Release() from the created thread without causing random segfaults.
		PyEval_InitThreads();
		#elif !defined(NSWIG)
		// If we want to use SWIG with another target language, we potentially need to add
		// code to ensure thread safety.
		#	error "Unknown SWIG target, thread safety for target language ignored!"
		#endif
		
		if(ret->contype == CON_WAIT) {
			rc = pthread_create(&ret->sproc->proct, NULL, _or_wait_proc, ret);
		} else if(ret->contype == CON_REPLY) {
			rc = pthread_create(&ret->sproc->proct, NULL, _or_reply_proc, ret);
		}
		
		if(rc) {
			printf("Error launching thread: %d\n", rc);
			abort();
		}
		break;
	}
	
	return ret;
}

void or_disconnect(struct or_socket_t *s) {
	int rc;
	
	if(s->sproc) {
		void *status;
		s->sproc->doExit = true;
		pthread_join(s->sproc->proct, &status);
	}
	
	rc = zmq_close(s->sock);
	if(rc == -1) {
		printf("Error closing socket: %s\n", zmq_strerror(errno));
		abort();
	}
	
	if(s->sproc) {
		free(s->sproc);
	}
	
	free(s->endpoint);
	free(s);
}

int set_socket_proc_cb(struct or_socket_t *s,
		const socket_proc_cb_t cb,
		const void *const swigcb) {
	switch(s->contype) {
	 case CON_PUSH:
	 case CON_REQUEST:
		return ORSOQS_ERR_BSO;
	
	 case CON_WAIT:
	 case CON_REPLY:
		s->sproc->proccb = cb;
		s->sproc->swigcb = (void*)swigcb;
		return 0;
	}
	
	return ORSOQS_ERR_UCR;
}

void *_or_wait_proc(void *arg) {
	struct or_socket_t *s = (struct or_socket_t *)arg;
	int rc;
	
	while(!s->sproc->doExit) {
		zmq_msg_t msg;
		zmq_msg_t rmsg;
		zmq_msg_init(&msg);
		
		rc = zmq_msg_recv(&msg, s->sock, 0);
		if(rc == -1) {
			if(errno == EAGAIN) {
				continue;
			} else {
				printf("Error receiving message: %s\n", zmq_strerror(errno));
				abort();
			}
		}
		
		char* out = NULL;
		uint64_t outl = s->sproc->proccb(zmq_msg_data(&msg),
				zmq_msg_size(&msg),
				&out,
				s->sproc->swigcb);
		
		zmq_msg_close(&msg);
		
		if(outl == 0 || out == NULL) {
			printf("Warning: Output buffer not allocated in callback\n");
			continue;
		}
		
		zmq_msg_init_data(&rmsg, out, outl, _or_zfree, NULL);
		rc = zmq_msg_send(&rmsg, s->sock, 0);
		
		if(rc == -1) {
			printf("Error sending message: %s\n", zmq_strerror(errno));
			abort();
		}
	}
	
	pthread_exit((void*)0);
}

void *_or_reply_proc(void *arg) {
	struct or_socket_t *s = (struct or_socket_t *)arg;
	int rc;
	
	while(!s->sproc->doExit) {
		zmq_msg_t msg;
		zmq_msg_t rmsg;
		zmq_msg_init(&msg);
		
		rc = zmq_msg_recv(&msg, s->sock, 0);
		if(rc == -1) {
			if(errno == EAGAIN) {
				continue;
			} else {
				printf("Error receiving message: %s\n", zmq_strerror(errno));
				abort();
			}
		}
		
		char* out = NULL;
		uint64_t outl = s->sproc->proccb(zmq_msg_data(&msg),
				zmq_msg_size(&msg),
				&out,
				s->sproc->swigcb);
		
		zmq_msg_close(&msg);
		
		if(outl == 0 || out == NULL) {
			printf("Warning: Output buffer not allocated in callback\n");
			continue;
		}
		
		zmq_msg_init_data(&rmsg, out, outl, _or_zfree, NULL);
		rc = zmq_msg_send(&rmsg, s->sock, 0);
		
		if(rc == -1) {
			printf("Error sending message: %s\n", zmq_strerror(errno));
			abort();
		}
	}
	
	pthread_exit((void*)0);
}

void _or_zfree(void *data, void *hint) {
	UNUSED(hint);
	
	free(data);
}
