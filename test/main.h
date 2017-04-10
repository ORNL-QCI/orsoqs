#ifndef LIBORSOQS_TEST_MAIN_H
#define LIBORSOQS_TEST_MAIN_H

#include "liborsoqs.h"
#include <check.h>
#include "client.h"
#include "socket.h"
#include "connection.h"

/**
 * \brief Current implemented test suites.
 */
enum test_suite {
	TS_CLIENT,
	TS_SOCKET,
	TS_CONNECTION,
};

/**
 * \brief Excute a test suite and return the number of failed tests.
 */
uint64_t test(enum test_suite ts);

/**
 * \brief Main entry point.
 */
int main();

#endif
