#ifndef LIBORSOQS_MSG_H
#define LIBORSOQS_MSG_H

#include <common.h>
#include "socket.h"
#include <zmq.h>

/**
 * \brief A message object that may either hold allocated memory or act as a reference to
 * an existing buffer.
 */
struct or_msg_t {
	char *data;
	int64_t size;
};

/**
 * \brief Create a message object.
 * 
 * \param	data	The input data.
 * \param	size	The size of the input data buffer.
 * 
 * \returns A message object that must be deleted with or_delete_msg().
 */
struct or_msg_t *or_create_msg(const char* const data,
		const unsigned int size);

/**
 * \brief Delete a message object.
 * 
 * \param	msg		The message object to delete.
 */
void or_delete_msg(struct or_msg_t *msg);

/**
 * \brief Send a message on a socket that is not passive.
 * 
 * \param	s		The socket object to send the message on.
 * \param	msg		The message to send.
 * 
 * \param Returns the received message or a numerical error value.
 */
struct or_msg_t *or_send_msg(struct or_socket_t *s,
		const struct or_msg_t *msg);

#endif
