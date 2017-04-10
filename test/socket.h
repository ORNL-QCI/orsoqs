#ifndef LIBORSOQS_TEST_SOCKET_H
#define LIBORSOQS_TEST_SOCKET_H

#include "main.h"

/**
 * \brief Client object used to create socket connections.
 */
struct or_client_t *client;

/**
 * \brief Setup before test suite begins.
 */
void socket_setup();

/**
 * \brief Teardown after test suite completes.
 */
void socket_teardown();

/**
 * \brief Create the test suite for socket.
 */
Suite *socket_suite(void);

#endif
