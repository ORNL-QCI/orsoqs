#ifndef LIBORSOQS_TEST_CONNECTION_H
#define LIBORSOQS_TEST_CONNECTION_H

#include "main.h"

/**
 * \brief Client object used to create socket connections.
 */
struct or_client_t *client;

/**
 * \brief Instance of a ZMQ context.
 */
void *zctx;

/**
 * \brief Condition flag to exit the threaded zmq server.
 */
volatile bool doExit;

/**
 * \brief Data used to check integrity of transmission.
 */
char *dataToReceive;

/**
 * \brief Data used to check integrity of transmission.
 */
char *dataToReply;

/**
 * \brief The endpoint we use in these tests.
 */
const char *endpoint;

/**
 * \brief Timeout for sending messages from zmq server.
 * 
 * \note Milliseconds.
 */
int zsndto;

/**
 * \brief Timeout for receiving messages from zmq server.
 * 
 * \note Milliseconds;
 */
int zrcvto;

/**
 * \brief Setup before test suite begins.
 */
void connection_setup();

/**
 * \brief Teardown after test suite completes.
 */
void connection_teardown();

/**
 * \brief Function launched in a pthread that acts as a basic ZMQ server.
 */
void *pt_zlisten(void *args);

/**
 * \brief Processing callback function.
 */
int64_t connection_proc_cb(const char *const in,
		int64_t ilen,
		char **out,
		const void *pycb);

/**
 * \brief Create the test suite for connection.
 */
Suite *connection_suite(void);

#endif
