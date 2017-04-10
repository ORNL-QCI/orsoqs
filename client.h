#ifndef LIBORSOQS_CLIENT_H
#define LIBORSOQS_CLIENT_H

#include <common.h>
#include <zmq.h>

/**
 * \brief A client object that sockets are connected through and represents a thread that
 * IO takes place on.
 * 
 * There should be only one client per application.
 */
struct or_client_t {
	/**
	 * \brief Pointer to the zmq context.
	 */
	void *zcontext;
};

/**
 * \brief Create a client object.
 * 
 * \returns A client object that must be deleted with or_delete_client().
 */
struct or_client_t *or_create_client();

/**
 * \brief Delete a client object.
 * 
 * \param	c		The client object to delete.
 */
void or_delete_client(struct or_client_t *c);

#endif
