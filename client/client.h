/*
 * client.h
 *
 *  Created on: Apr 13, 2015
 *      Author: wojtek
 */

#ifndef uthash_fatal
#define uthash_fatal(msg) return -1;
#endif

#include "uthash.h"
#include "coap/net.h"
#include "coap/block.h"
#include "coap_list.h"

#ifndef COAP_CLIENT_CLIENT_H_
#define COAP_CLIENT_CLIENT_H_

typedef enum {
	BLOCK = 1,
	SUBSCRIBE = 2,
	UNSUBSCRIBE = 4,
} request_flags_t;

#define AUTO_TOKEN_LENGTH    4
#define MAX_TOKEN_LENGTH     8

struct _coap_request_t;
struct _coap_response_t;

typedef int (*coap_client_cb_t)(struct _coap_request_t *, struct _coap_response_t*);

typedef struct {
	coap_context_t *coap;
	coap_endpoint_t *ep;
	struct _coap_request_t *requests;

	int (*block_write)(struct _coap_response_t *response, coap_pdu_t *pdu, coap_block_t *block);
	int (*block_read)(struct _coap_request_t *request, coap_pdu_t *pdu, coap_block_t *block);

} coap_client_t;

// This is a future object.
typedef struct _coap_response_t {
	uint8_t code;            /* response code */
	uint8_t content_type;    /* response content type */
	void *data;              /* response payload data */
	size_t length;           /* response lenght */
} coap_response_t;

typedef struct _coap_request_t {
	coap_endpoint_t *ep;
	coap_client_t *client;
	coap_address_t dst;
	coap_list_t *optlist;
	coap_uri_t uri;

	uint8_t method;
	uint8_t type;
	uint8_t token[MAX_TOKEN_LENGTH];
	uint8_t token_length;

	void *data;
	size_t length;

	int block_type;
	coap_block_t block;

	coap_response_t *response;
	coap_client_cb_t cb;

	void *pdata;

	UT_hash_handle hh;
} coap_request_t;

int coap_request_set_uri(coap_request_t *request, const char *arg, int is_proxy_uri);

static inline void coap_request_set_token(coap_request_t *request, uint8_t *token, int length)
{
	memcpy(request->token, token, MIN(length, MAX_TOKEN_LENGTH));
}

static inline coap_request_t *coap_client_find_request(coap_client_t *client, uint8_t *token, int token_length)
{
	coap_request_t *found = NULL;
	HASH_FIND(hh, client->requests, token, token_length, found);
	return found;
}

void coap_client_init(coap_client_t *client, coap_context_t *ctx, coap_endpoint_t *ep);
void coap_client_deinit(coap_client_t *client);

coap_request_t *coap_client_new_request(unsigned char method, unsigned char type);
void coap_client_delete_request(coap_request_t *r);

int coap_client_send_request(coap_client_t *client, coap_request_t *request, coap_response_t *response, coap_address_t *dst);

coap_request_t *coap_client_get(coap_client_t *client, const char *uri, coap_client_cb_t cb);
coap_request_t *coap_client_put(coap_client_t *client, const char *uri, void *data, size_t len, coap_client_cb_t cb);
coap_request_t *coap_client_post(coap_client_t *client, const char *uri, void *data, size_t len, coap_client_cb_t cb);

#endif /* COAP_CLIENT_CLIENT_H_ */
