#include "client.h"

START_TEST(client_create_delete) {
	struct or_client_t *client;
	bool error;
	
	client = or_create_client();
	error = or_check_error((int64_t)client);
	
	ck_assert_msg(!error, "Create client call returned error");
	
	or_delete_client(client);
}
END_TEST

Suite *client_suite(void) {
	Suite *s;
	TCase *tc_create_delete;
	
	s = suite_create("client");
	
	tc_create_delete = tcase_create("create_delete");
	tcase_add_test(tc_create_delete, client_create_delete);
	suite_add_tcase(s, tc_create_delete);
	
	return s;
}
