#include "msg.h"

struct or_msg_t *or_create_msg(const char* const data,
		const unsigned int size) {
	struct or_msg_t *msg = (struct or_msg_t*)malloc(sizeof(struct or_msg_t));
	
	msg->data = malloc(size);
	memcpy(msg->data, data, size);
	
	msg->size = size;
	
	return msg;
}

void or_delete_msg(struct or_msg_t *msg) {
	free(msg->data);
	free(msg);
}

struct or_msg_t *or_send_msg(struct or_socket_t *s,
		const struct or_msg_t *msg) {
	if(s->contype == CON_WAIT || s->contype == CON_REPLY) {
		return (struct or_msg_t*)ORSOQS_ERR_BSO;
	}
	
	int rc;
	struct or_msg_t *ret;
	zmq_msg_t zmsg;
	
	rc = zmq_send_const(s->sock, msg->data, msg->size, 0);
	if(rc == -1) {
		printf("Error sending message: %s\n", zmq_strerror(errno));
		abort();
	}
	
	rc = zmq_msg_init(&zmsg);
	if(rc == -1) {
		printf("Error initializing message: %s\n", zmq_strerror(errno));
		abort();
	}
	
	rc = zmq_msg_recv(&zmsg, s->sock, 0);
	if(rc == -1) {
		printf("Error receiving message: %s\n", zmq_strerror(errno));
		abort();
	}
	
	switch(s->contype) {
	 case CON_PUSH:
		ret = or_create_msg(zmq_msg_data(&zmsg), zmq_msg_size(&zmsg));
		break;
	
	 case CON_REQUEST:
		ret = or_create_msg(zmq_msg_data(&zmsg), zmq_msg_size(&zmsg));
		break;
	
	 case CON_WAIT:
	 case CON_REPLY:
	 default:
		ret = (struct or_msg_t*)ORSOQS_ERR_UCR;
		break;
	}
	
	zmq_msg_close(&zmsg);
	return ret;
}
