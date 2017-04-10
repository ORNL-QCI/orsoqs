#ifndef LIBORSOQS_SOCKET_H
#define LIBORSOQS_SOCKET_H

#include <common.h>
#include "error.h"
#include "client.h"
#include <pthread.h>
#include <zmq.h>

/**
 * \brief The read timeout used in the processing functions.
 */
int _or_socket_proc_rdtimeo;

/**
 * \brief The write timeout used in the processing functions.
 */
int _or_socket_proc_wttimeo;

/**
 * \brief The types of connections a socket can make.
 */
enum or_contype_t {
	/**
	 * \brief Send a message to the middleware with the expectation of a receipt.
	 */
	CON_PUSH,
	
	/**
	 * \brief Passively wait for messages from the middleware and send a receipt.
	 */
	CON_WAIT,
	
	/**
	 * \brief Send a request to the middleware with the expectation of processed reply.
	 */
	CON_REQUEST,
	
	/**
	 * \brief Passively wait for requests from the middleware and reply.
	 */
	CON_REPLY,
};

/**
 * \brief Function pointer type to a processing function called when sockets of certain
 * connection types receive data.
 * 
 * \param	in		The input buffer.
 * \param	ilen	The size of the input buffer.
 * \param	out		todo
 * \param	pycb	todo
 * 
 * \returns The size of the out buffer that was allocated.
 */
typedef int64_t (*socket_proc_cb_t)(const char *const in,
		int64_t ilen,
		char **out,
		const void *pycb);

/**
 * \brief The processing elements for a passive socket.
 */
struct _or_socket_proc_t {
	/**
	 * \brief The processing callback function pointer.
	 */
	socket_proc_cb_t proccb;
	
	#ifdef SWIG_PYTHON
	/**
	 * \brief The processing callback function pointer to the Python function.
	 */
	void *swigcb;
	#endif
	
	/**
	 * \brief The thread that we wait for data on.
	 */
	pthread_t proct;
	
	/**
	 * \brief Terminate flag.
	 */
	volatile bool doExit;
};

/**
 * \brief A socket object that is a connection to the middleware.
 */
struct or_socket_t {
	/**
	 * \brief The client object the socket is connected through.
	 */
	struct or_client_t *client;
	
	/**
	 * \brief The endpoint the socket is connected to.
	 */
	char *endpoint;
	
	/**
	 * \brief Pointer to the zmq socket.
	 */
	void *sock;
	
	/**
	 * \brief If the socket is passive, this is a pointer to our socket processing struct.
	 */
	struct _or_socket_proc_t* sproc;
	
	/**
	 * \brief The type of connection this is.
	 */
	enum or_contype_t contype;
};

/**
 * \brief Create a connection to the middleware.
 * 
 * \param	client	A client object the socket will be created on.
 * \param	loc		The endpoint location to connect to.
 * \param	ct		The type of connection to establish.
 * 
 * \returns A socket object that must be deleted with or_disconnect().
 */
struct or_socket_t *or_connect(struct or_client_t *c,
		const char *const loc,
		const enum or_contype_t ct);

/**
 * \brief Disconnect a connection to the middleware.
 * 
 * \param	s		Socket object to disconnect and destroy.
 */
void or_disconnect(struct or_socket_t *s);

/**
 * \brief Register a processing callback function for a socket.
 *
 * \param	s		The socket to register the processing callback for.
 * \param	cb		The processing callback function.
 * \param	swigcb	The processing callback function used in swig interfaces only. If the
 * 					callback function is in C, then this can be 0.
 * 
 * \returns An integer value other than 0 if there was an error.
 */
int set_socket_proc_cb(struct or_socket_t *s,
		const socket_proc_cb_t cb,
		const void *const swigcb);

/**
 * \brief The function launched on a thread for a socket with a wait connection type.
 * 
 * \param	arg		The socket in use.
 * 
 * \returns The typical pthread 0.
 */
void *_or_wait_proc(void *arg);

/**
 * \brief The function launched on a thread for a socket with a reply connection type.
 * 
 * \param	arg		The socket in use.
 * 
 * \returns The typical pthread 0.
 */
void *_or_reply_proc(void *arg);

/**
 * \brief A helper function used with zmq when deallocating buffers passed as references
 * into messages. This gets called when zmq has pushed the message out and no longer
 * requires the data.
 * 
 * \param	data	The data buffer supplied to zmq.
 * \param	hint	The data buffer hint supplied to zmq.
 */
void _or_zfree(void *data, void* hint);

#endif
