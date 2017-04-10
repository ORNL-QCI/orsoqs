#include "connection.h"

struct or_client_t *client;
void *zctx;
volatile bool doExit = false;
char *dataToReceive = "Hello world!";
char *dataToReply = "Hello earthlings!";
const char *endpoint = "tcp://127.0.0.1:5553";
int zsndto = 100;
int zrcvto = 100;

void connection_setup() {
	client = or_create_client();
	zctx = zmq_ctx_new();
}

void connection_teardown() {
	or_delete_client(client);
	zmq_ctx_destroy(zctx);
}

void *pt_zlisten(void *args) {
	UNUSED(args);
	
	void *zsock;
	int rc;
	
	zsock = zmq_socket(zctx, ZMQ_PAIR);
	ck_assert_msg(zsock != NULL, "Error creating zmq socket");
	
	zmq_setsockopt(zsock,
			ZMQ_RCVTIMEO,
			&zsndto,
			sizeof(zsndto));
	zmq_setsockopt(zsock,
			ZMQ_SNDTIMEO,
			&zrcvto,
			sizeof(zrcvto));
	
	rc = zmq_bind(zsock, endpoint);
	ck_assert_msg(rc != -1, "Error binding on zmq socket");
	
	while(!doExit) {
		zmq_msg_t zmsg;
		rc = zmq_msg_init(&zmsg);
		ck_assert_msg(rc != -1, "Error initializing zmq message");
		
		rc = zmq_msg_recv(&zmsg, zsock, 0);
		
		if(rc == -1) {
			if(errno == EAGAIN) {
				continue;
			} else {
				ck_assert_msg(rc != -1, "Error receiving zmq message");
			}
		}
		
		ck_assert_msg(memcmp(zmq_msg_data(&zmsg), dataToReceive, strlen(dataToReceive)) == 0,
				"zmq received unexpected data.");
		
		zmq_msg_close(&zmsg);
		
		rc = zmq_send_const(zsock, dataToReply, strlen(dataToReply), 0);
		
		ck_assert_msg(rc != -1, "Error sending zmq message");
	}
	
	zmq_unbind(zsock, endpoint);
	rc = zmq_close(zsock);
	ck_assert_msg(rc != -1, "Error closing zmq socket");
	
	pthread_exit((void*)0);
}

int64_t connection_proc_cb(const char *const in,
		int64_t ilen,
		char **out,
		const void *pycb) {
	UNUSED(pycb);
	
	ck_assert_msg(ilen = strlen(dataToReceive),
				"callback received unexpected data.");
	
	ck_assert_msg(memcmp(in, dataToReceive, strlen(dataToReceive)) == 0,
				"callback received unexpected data.");
	
	*out = malloc(strlen(dataToReply));
	memcpy(*out, dataToReply, strlen(dataToReply));
	
	return strlen(dataToReply);
}

START_TEST(connection_proc_push) {
	pthread_t proct;
	int rc;
	bool error;
	void *status;
	struct or_msg_t *msg;
	struct or_msg_t *rmsg;
	struct or_socket_t *socket;
	
	rc = pthread_create(&proct, NULL, pt_zlisten, NULL);
	ck_assert_msg(!rc, "Error launching thread");
	
	socket = or_connect(client, endpoint, CON_PUSH);
	error = or_check_error((int64_t)socket);
	ck_assert_msg(!error, "Create socket call for push returned error");
	
	msg = or_create_msg(dataToReceive, strlen(dataToReceive));
	
	rmsg = or_send_msg(socket, msg);
	
	ck_assert_msg(memcmp(rmsg->data, dataToReply, strlen(dataToReply)) == 0,
			"client Received unxpected data.");
	
	or_delete_msg(msg);
	or_delete_msg(rmsg);
	or_disconnect(socket);
	
	doExit = true;
	pthread_join(proct, &status);
}
END_TEST

START_TEST(connection_proc_wait) {
	int rc;
	void *zsock;
	zmq_msg_t zmsg;
	struct or_socket_t *socket;
	bool error;
	
	socket = or_connect(client, endpoint, CON_WAIT);
	error = or_check_error((int64_t)socket);
	ck_assert_msg(!error, "Create socket call for push returned error");
	
	set_socket_proc_cb(socket, connection_proc_cb, NULL);
	
	zsock = zmq_socket(zctx, ZMQ_PAIR);
	ck_assert_msg(zsock != NULL, "Error creating zmq socket");
	
	rc = zmq_connect(zsock, endpoint);
	ck_assert_msg(rc != -1, "Error connecting on zmq socket");
	
	rc = zmq_send_const(zsock, dataToReceive, strlen(dataToReceive), 0);
	
	ck_assert_msg(rc != -1, "Error sending zmq message");
	
	zmq_msg_init(&zmsg);
	rc = zmq_msg_recv(&zmsg, zsock, 0);
	
	ck_assert_msg(rc != -1, "Error receiving zmq message");
	
	ck_assert_msg(zmq_msg_size(&zmsg) == strlen(dataToReply),
				"zmq received unexpected data.");
	
	ck_assert_msg(memcmp(zmq_msg_data(&zmsg), dataToReply, strlen(dataToReply)) == 0,
				"zmq received unexpected data.");
	
	zmq_msg_close(&zmsg);
	zmq_disconnect(zsock, endpoint);
	rc = zmq_close(zsock);
	ck_assert_msg(rc != -1, "Error closing zmq socket");
	or_disconnect(socket);
}
END_TEST

START_TEST(connection_proc_request) {
	pthread_t proct;
	int rc;
	void *status;
	struct or_msg_t *msg;
	struct or_msg_t *rmsg;
	struct or_socket_t *socket;
	bool error;
	
	rc = pthread_create(&proct, NULL, pt_zlisten, NULL);
	ck_assert_msg(!rc, "Error launching thread");
	
	socket = or_connect(client, endpoint, CON_REQUEST);
	error = or_check_error((int64_t)socket);
	ck_assert_msg(!error, "Create socket call for request returned error");
	
	msg = or_create_msg(dataToReceive, strlen(dataToReceive));
	
	rmsg = or_send_msg(socket, msg);
	
	ck_assert_msg(memcmp(rmsg->data, dataToReply, strlen(dataToReply)) == 0,
			"client Received unxpected data.");
	
	or_delete_msg(msg);
	or_delete_msg(rmsg);
	or_disconnect(socket);
	
	doExit = true;
	pthread_join(proct, &status);
}
END_TEST

START_TEST(connection_proc_reply) {
	int rc;
	void *zsock;
	zmq_msg_t zmsg;
	struct or_socket_t *socket;
	bool error;
	
	socket = or_connect(client, endpoint, CON_REPLY);
	error = or_check_error((int64_t)socket);
	ck_assert_msg(!error, "Create socket call for push returned error");
	
	set_socket_proc_cb(socket, connection_proc_cb, NULL);
	
	zsock = zmq_socket(zctx, ZMQ_PAIR);
	ck_assert_msg(zsock != NULL, "Error creating zmq socket");
	
	rc = zmq_connect(zsock, endpoint);
	ck_assert_msg(rc != -1, "Error connecting on zmq socket");
	
	rc = zmq_send_const(zsock, dataToReceive, strlen(dataToReceive), 0);
	
	ck_assert_msg(rc != -1, "Error sending zmq message");
	
	zmq_msg_init(&zmsg);
	rc = zmq_msg_recv(&zmsg, zsock, 0);
	
	ck_assert_msg(rc != -1, "Error receiving zmq message");
	
	ck_assert_msg(zmq_msg_size(&zmsg) == strlen(dataToReply),
				"zmq received unexpected data.");
	
	ck_assert_msg(memcmp(zmq_msg_data(&zmsg), dataToReply, strlen(dataToReply)) == 0,
				"zmq received unexpected data.");
	
	zmq_msg_close(&zmsg);
	zmq_disconnect(zsock, endpoint);
	rc = zmq_close(zsock);
	ck_assert_msg(rc != -1, "Error closing zmq socket");
	or_disconnect(socket);
}
END_TEST

Suite *connection_suite(void) {
	Suite *s;
	TCase *tc_proc_push;
	TCase *tc_proc_wait;
	TCase *tc_proc_request;
	TCase *tc_proc_reply;
	
	s = suite_create("connection");
	
	tc_proc_push = tcase_create("proc_push");
	tcase_add_checked_fixture(tc_proc_push, connection_setup, connection_teardown);
	tcase_add_test(tc_proc_push, connection_proc_push);
	suite_add_tcase(s, tc_proc_push);
	
	tc_proc_wait = tcase_create("proc_wait");
	tcase_add_checked_fixture(tc_proc_wait, connection_setup, connection_teardown);
	tcase_add_test(tc_proc_wait, connection_proc_wait);
	suite_add_tcase(s, tc_proc_wait);
	
	tc_proc_request = tcase_create("proc_request");
	tcase_add_checked_fixture(tc_proc_request, connection_setup, connection_teardown);
	tcase_add_test(tc_proc_request, connection_proc_request);
	suite_add_tcase(s, tc_proc_request);
	
	tc_proc_reply = tcase_create("proc_reply");
	tcase_add_checked_fixture(tc_proc_reply, connection_setup, connection_teardown);
	tcase_add_test(tc_proc_reply, connection_proc_reply);
	suite_add_tcase(s, tc_proc_reply);
	
	return s;
}
