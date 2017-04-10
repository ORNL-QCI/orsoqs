#include "client.h"

struct or_client_t *or_create_client() {
	struct or_client_t *client = (struct or_client_t*)malloc(sizeof(struct or_client_t));
	
	client->zcontext = zmq_ctx_new();
	
	return client;
}

void or_delete_client(struct or_client_t *c) {
	zmq_ctx_destroy(c->zcontext);
	
	free(c);
}
