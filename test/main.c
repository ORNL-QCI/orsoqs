#include "main.h"

uint64_t test(enum test_suite ts) {
	uint64_t failCount;
	Suite *s;
	SRunner *sr;
	
	switch(ts) {
	 case TS_CLIENT:
		s = client_suite();
		break;
	
	 case TS_SOCKET:
		s = socket_suite();
		break;
	
	 case TS_CONNECTION:
		s = connection_suite();
		break;
	
	 default:
		printf("Unimplemented test suite");
		exit(EXIT_FAILURE);
	}
	
	sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	failCount = srunner_ntests_failed(sr);
	srunner_free(sr);
	
	return failCount;
}

int main(int argc, char *argv[]) {
	uint64_t failCount = 0;
	
	if(argc != 2) {
		printf("usage: %s [test]\n", argv[0]);
		printf("available tests: client, socket, connection, all\n");
		return EXIT_FAILURE;
	}
	
	if(strcmp(argv[1], "client") == 0) {
		failCount += test(TS_CLIENT);
		
	} else if(strcmp(argv[1], "socket") == 0) {
		failCount += test(TS_SOCKET);
		
	} else if(strcmp(argv[1], "connection") == 0) {
		failCount += test(TS_CONNECTION);
		
	} else if(strcmp(argv[1], "all") == 0) {
		failCount += test(TS_CLIENT);
		failCount += test(TS_SOCKET);
		failCount += test(TS_CONNECTION);
		
	} else {
		printf("unknown test: %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	
	return (failCount == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
