#include "socket.h"

struct or_client_t *client;

void socket_setup() {
	client = or_create_client();
}

void socket_teardown() {
	or_delete_client(client);
}

START_TEST(socket_create_delete_push) {
	struct or_socket_t *socket;
	bool error;
	
	socket = or_connect(client, "tcp://127.0.0.1:12345", CON_PUSH);
	error = or_check_error((int64_t)socket);
	
	ck_assert_msg(!error, "Create socket call for push returned error");
	
	or_disconnect(socket);
}
END_TEST

START_TEST(socket_create_delete_wait) {
	struct or_socket_t *socket;
	bool error;
	void *zcontext;
	void *zsocket;
	int rc;
	
	zcontext = zmq_ctx_new();
	zsocket = zmq_socket(zcontext, ZMQ_REQ);
	if(zsocket == NULL) {
		printf("%s", zmq_strerror(errno));
		abort();
	}
	rc = zmq_connect(zsocket, "tcp://127.0.0.1:12345");
	if(rc) {
		printf("%s", zmq_strerror(errno));
		abort();
	}
	
	socket = or_connect(client, "tcp://127.0.0.1:12345", CON_WAIT);
	error = or_check_error((int64_t)socket);
	
	ck_assert_msg(!error, "Create socket call for wait returned error");
	
	or_disconnect(socket);
	rc = zmq_close(zsocket);
	if(rc) {
		printf("%s", zmq_strerror(errno));
		abort();
	}
	zmq_ctx_destroy(zcontext);
}
END_TEST

START_TEST(socket_create_delete_request) {
	struct or_socket_t *socket;
	bool error;
	
	socket = or_connect(client, "tcp://127.0.0.1:12345", CON_REQUEST);
	error = or_check_error((int64_t)socket);
	
	ck_assert_msg(!error, "Create socket call for request returned error");
	
	or_disconnect(socket);
}
END_TEST

START_TEST(socket_create_delete_reply) {
	struct or_socket_t *socket;
	bool error;
	void *zcontext;
	void *zsocket;
	int rc;
	
	zcontext = zmq_ctx_new();
	zsocket = zmq_socket(zcontext, ZMQ_REQ);
	if(zsocket == NULL) {
		printf("%s", zmq_strerror(errno));
		abort();
	}
	rc = zmq_connect(zsocket, "tcp://127.0.0.1:12345");
	if(rc) {
		printf("%s", zmq_strerror(errno));
		abort();
	}
	
	socket = or_connect(client, "tcp://127.0.0.1:12345", CON_REPLY);
	error = or_check_error((int64_t)socket);
	
	ck_assert_msg(!error, "Create socket call for reply returned error");
	
	or_disconnect(socket);
	rc = zmq_close(zsocket);
	if(rc) {
		printf("%s", zmq_strerror(errno));
		abort();
	}
	zmq_ctx_destroy(zcontext);
}
END_TEST

Suite *socket_suite(void) {
	Suite *s;
	TCase *tc_create_delete_push;
	TCase *tc_create_delete_wait;
	TCase *tc_create_delete_request;
	TCase *tc_create_delete_reply;
	
	s = suite_create("socket");
	
	tc_create_delete_push = tcase_create("create_delete_push");
	tcase_add_checked_fixture(tc_create_delete_push, socket_setup, socket_teardown);
	tcase_add_test(tc_create_delete_push, socket_create_delete_push);
	suite_add_tcase(s, tc_create_delete_push);
	
	tc_create_delete_wait = tcase_create("create_delete_wait");
	tcase_add_checked_fixture(tc_create_delete_wait, socket_setup, socket_teardown);
	tcase_add_test(tc_create_delete_wait, socket_create_delete_wait);
	suite_add_tcase(s, tc_create_delete_wait);
	
	tc_create_delete_request = tcase_create("create_delete_request");
	tcase_add_checked_fixture(tc_create_delete_request, socket_setup, socket_teardown);
	tcase_add_test(tc_create_delete_request, socket_create_delete_request);
	suite_add_tcase(s, tc_create_delete_request);
	
	tc_create_delete_reply = tcase_create("create_delete_reply");
	tcase_add_checked_fixture(tc_create_delete_reply, socket_setup, socket_teardown);
	tcase_add_test(tc_create_delete_reply, socket_create_delete_reply);
	suite_add_tcase(s, tc_create_delete_reply);
	
	return s;
}
