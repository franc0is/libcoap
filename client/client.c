#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

//#include "utils.h"
//#include "clock.h"
//#include "mbuf.h"

#include "coap/debug.h"
#include "coap/coap_list.h"
#include "coap/net.h"
#include "coap/address.h"
#include "coap/resource.h"
#include "coap/block.h"
#include "coap/pdu.h"
#include "coap/mem.h"
//#include "coap_utils.h"

#include "client.h"

extern int resolve_address(const str *addr, coap_address_t *dst);

static int check_token(coap_request_t *request, coap_pdu_t *received)
{
	 return received->hdr->token_length == request->token_length &&
			memcmp(received->hdr->token, request->token, request->token_length) == 0;
}

static int order_opts(void *a, void *b)
{
	if (!a || !b)
		return a < b ? -1 : 1;

	if (COAP_OPTION_KEY(*(coap_option *)a) < COAP_OPTION_KEY(*(coap_option * )b))
		return -1;

	return COAP_OPTION_KEY(*(coap_option *)a) == COAP_OPTION_KEY(*(coap_option * )b);
}

coap_list_t *
new_option_node(unsigned short key, unsigned int length, unsigned char *data)
{
	coap_list_t *node;

	node = coap_malloc(sizeof(coap_list_t) + sizeof(coap_option) + length);

	if (node) {
		coap_option *option = (coap_option *) (node->data);
		COAP_OPTION_KEY(*option) = key;
		COAP_OPTION_LENGTH(*option) = length;
		memcpy(COAP_OPTION_DATA(*option), data, length);
	} else {
		critical("new_option_node: malloc\n");
	}

	return node;
}

static void set_blocksize(coap_request_t *request)
{
	unsigned char buf[4]; /* hack: temporarily take encoded bytes */
	unsigned short opt;

	if (request->method != COAP_REQUEST_DELETE) {
		opt = (request->method == COAP_REQUEST_GET ? COAP_OPTION_BLOCK2 : COAP_OPTION_BLOCK1);
		coap_insert(&request->optlist, new_option_node(opt, coap_encode_var_bytes(buf, (request->block.num << 4 | request->block.szx)), buf));
	}
}

int coap_request_set_uri(coap_request_t *request, const char *arg, int is_proxy_uri)
{
	unsigned char portbuf[2];
	unsigned char _buf[128];
	unsigned char *buf = _buf;
	size_t buflen;
	int res;

	if (is_proxy_uri) { /* create Proxy-Uri from argument */
		size_t len = strlen(arg);
		while (len > 270) {
			coap_insert(&request->optlist, new_option_node(COAP_OPTION_PROXY_URI, 270, (unsigned char *) arg));
			len -= 270;
			arg += 270;
		}
		coap_insert(&request->optlist, new_option_node(COAP_OPTION_PROXY_URI, len, (unsigned char *) arg));

	} else { /* split arg into Uri-* options */
		if (coap_split_uri(arg, strlen(arg), &request->uri) < 0) {
			return 0;
		}

		if (request->uri.port != COAP_DEFAULT_PORT) {
			coap_insert(&request->optlist, new_option_node(COAP_OPTION_URI_PORT, coap_encode_var_bytes(portbuf, request->uri.port), portbuf));
		}

		if (request->uri.path.length) {
			buflen = sizeof(_buf);
			res = coap_split_path(request->uri.path.s, request->uri.path.length, buf, &buflen);
			while (res--) {
				coap_insert(&request->optlist, new_option_node(COAP_OPTION_URI_PATH, COAP_OPT_LENGTH(buf), COAP_OPT_VALUE(buf)));
				buf += COAP_OPT_SIZE(buf);
			}
		}

		if (request->uri.query.length) {
			buflen = sizeof(_buf);
			buf = _buf;
			res = coap_split_query(request->uri.query.s, request->uri.query.length, buf, &buflen);
			while (res--) {
				coap_insert(&request->optlist, new_option_node(COAP_OPTION_URI_QUERY, COAP_OPT_LENGTH(buf), COAP_OPT_VALUE(buf)));
				buf += COAP_OPT_SIZE(buf);
			}
		}
	}
	return 1;
}

//coap_tid_t clear_obs(coap_client_t *client, const coap_address_t *remote)
//{
//	coap_pdu_t *pdu;
//	coap_list_t *option;
//	coap_tid_t tid = COAP_INVALID_TID;
//	unsigned char buf[2];
//
//	/* create bare PDU w/o any option  */
//	pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_GET, coap_new_message_id(client->coap), COAP_MAX_PDU_SIZE);
//
//	if (!pdu) {
//		return tid;
//	}
//
//	if (!coap_add_token(pdu, the_token.length, the_token.s)) {
//		error("cannot add token");
//		goto error;
//	}
//
//	for (option = optlist; option; option = option->next) {
//		if (COAP_OPTION_KEY(*(coap_option *)option->data) == COAP_OPTION_URI_HOST) {
//			if (!coap_add_option(pdu, COAP_OPTION_KEY(*(coap_option * )option->data), COAP_OPTION_LENGTH(*(coap_option * )option->data),
//					COAP_OPTION_DATA(*(coap_option * )option->data))) {
//				goto error;
//			}
//			break;
//		}
//	}
//
//	if (!coap_add_option(pdu, COAP_OPTION_OBSERVE, coap_encode_var_bytes(buf, COAP_OBSERVE_CANCEL), buf)) {
//		error("cannot add option Observe: %u", COAP_OBSERVE_CANCEL);
//		goto error;
//	}
//
//	for (option = optlist; option; option = option->next) {
//		switch (COAP_OPTION_KEY(*(coap_option * )option->data)) {
//			case COAP_OPTION_URI_PORT:
//			case COAP_OPTION_URI_PATH:
//			case COAP_OPTION_URI_QUERY:
//				if (!coap_add_option(pdu, COAP_OPTION_KEY(*(coap_option * )option->data), COAP_OPTION_LENGTH(*(coap_option * )option->data),
//						COAP_OPTION_DATA(*(coap_option * )option->data))) {
//					goto error;
//				}
//				break;
//			default:
//				;
//		}
//	}
//
//	coap_show_pdu(pdu);
//
//	if (pdu->hdr->type == COAP_MESSAGE_CON)
//		tid = coap_send_confirmed(client->coap, client->ep, remote, pdu);
//	else
//		tid = coap_send(client->coap, client->ep, remote, pdu);
//
//	if (tid == COAP_INVALID_TID) {
//		debug("clear_obs: error sending new request");
//		coap_delete_pdu(pdu);
//	} else if (pdu->hdr->type != COAP_MESSAGE_CON)
//		coap_delete_pdu(pdu);
//
//	return tid;
//	error:
//
//	coap_delete_pdu(pdu);
//	return tid;
//}

/**
 * Create PDU from request.
 */
static coap_pdu_t *create_pdu(coap_request_t *request)
{
	coap_pdu_t *pdu;

	pdu = coap_new_pdu();
	if (!pdu) {
		critical("Failed to create request");
		return NULL;
	}

	pdu->hdr->type = request->type;
	pdu->hdr->id = coap_new_message_id(request->client->coap);
	pdu->hdr->code = request->method;
	pdu->hdr->token_length = request->token_length;
	coap_add_token(pdu, pdu->hdr->token_length, request->token);

	return pdu;
}

static int send_request(coap_client_t *client, coap_request_t *request, coap_response_t *response, coap_address_t *dst, coap_pdu_t *pdu)
{
	coap_tid_t tid;

	// We add request to the hash map before sending it. This is because it may
	// happen that the current thread will be preempted by network thread when an
	// acknowledgment is received. Since the response handler lookups the request this
	// could lead to bad things.

	coap_request_t *found = NULL;
	HASH_FIND(hh, client->requests, request->token, request->token_length, found);

	if (!found) {
		HASH_ADD_KEYPTR(hh, client->requests, request->token, request->token_length, request);
		request->client = client;
		request->response = response;
		response->code = 0;
	}

	// Send request
	if (pdu->hdr->type == COAP_MESSAGE_CON) {
		tid = coap_send_confirmed(client->coap, client->ep, dst, pdu);
	} else {
		tid = coap_send(client->coap, client->ep, dst, pdu);
	}

	if (tid == COAP_INVALID_TID) {
		coap_delete_pdu(pdu);
		HASH_DEL(client->requests, request);
		return 0;
	}

	if (pdu->hdr->type != COAP_MESSAGE_CON) {
		coap_delete_pdu(pdu);
		return 0;
	}

#ifndef NDEBUG
	char toks[request->token_length*2 + 1];
	memset(toks, 0, sizeof(toks));
//	htos(request->token, request->token_length, toks, sizeof(toks));
	info("Request sent (id:%d tk:%s)", tid, toks);
#endif

	return 1;
}

static int
request_block(coap_request_t *request)
{
	unsigned char buf[4];
	coap_list_t *option;

	coap_pdu_t *pdu = create_pdu(request);
	if (!pdu) {
		return 0;
	}

	//debug("found the M bit, block size is %u, block nr. %u\n", COAP_OPT_BLOCK_SZX(block_opt), coap_opt_block_num(block_opt));

	/* add URI components from optlist */
	for (option = request->optlist; option; option = option->next) {
		switch (COAP_OPTION_KEY(*(coap_option * )option->data)) {
			case COAP_OPTION_URI_HOST:
			case COAP_OPTION_URI_PORT:
			case COAP_OPTION_URI_PATH:
			case COAP_OPTION_URI_QUERY:
				coap_add_option(pdu, COAP_OPTION_KEY(*(coap_option * )option->data), COAP_OPTION_LENGTH(*(coap_option * )option->data),
						COAP_OPTION_DATA(*(coap_option * )option->data));
				break;
			default:
				break; /* skip other options */
		}
	}

	/* finally add updated block option from response, clear M bit */
	/* blocknr = (blocknr & 0xfffffff7) + 0x10; */
	debug("query block %d\n", request->block.num);
	coap_add_option(pdu, request->block_type, coap_encode_var_bytes(buf, (request->block.num << 4) | request->block.szx), buf);

	return send_request(request->client, request, request->response, &request->dst, pdu);
}

static void handle_205(coap_client_t *client, coap_request_t *request, coap_pdu_t *received)
{
	coap_opt_t *opt;
	coap_opt_iterator_t oi;

	coap_option_iterator_init(received, &oi, COAP_OPT_ALL);

	while ((opt = coap_option_next(&oi))) {
		switch (oi.type) {
			case COAP_OPTION_CONTENT_FORMAT:
				request->response->content_type = *(uint8_t *)coap_opt_value(opt);
				break;
			case COAP_OPTION_OBSERVE:
				break;
			case COAP_OPTION_BLOCK2: {
			    request->block.szx = COAP_OPT_BLOCK_SZX(opt);
			    if (COAP_OPT_BLOCK_MORE(opt)) {
			    	request->block.m = 1;
			    }
			    request->block.num = coap_opt_block_num(opt);

 			    info("Received block (NUM:%d, M:%d)\n", request->block.num, request->block.m);
				/* TODO: check if we are looking at the correct block number */
 			    if (coap_has_data(received) && client->block_write) {
					client->block_write(request->response, received, &request->block);
 			    }
				break;
			}
			default:
				break;
		}
	}

	if (request->block.m) {
		request->block_type = COAP_OPTION_BLOCK2;
		request->block.num += 1;
		request_block(request);
	} else {
		//request->response->data = coap_get_mbuf(received);
		request->response->code = received->hdr->code;
		// copy received data
		if (coap_has_data(received)) {
			size_t len;
			unsigned char *data;
			coap_get_data(received, &len, &data);
			request->response->data = coap_malloc(len);
			if (request->response->data) {
				memcpy(request->response->data, data, len);
				request->response->length = len;
			}
		}
		if (request->cb) {
			request->cb(request, request->response);
			coap_client_delete_request(request);
		}
	}
}

static void response_handler(
		struct coap_context_t *ctx,
		const coap_endpoint_t *ep,
		const coap_address_t *remote,
		coap_pdu_t *sent,
		coap_pdu_t *received,
		const coap_tid_t id)
{
	coap_client_t *client = (coap_client_t *)ctx->pdata;

	assert(client);

	char toks[received->hdr->token_length*2 + 1];
	memset(toks, 0, sizeof(toks));
//	htos(received->hdr->token, received->hdr->token_length, toks, sizeof(toks));

	coap_request_t *request = NULL;
	HASH_FIND(hh, client->requests, received->hdr->token, received->hdr->token_length, request);
	if (!request) {
		warn("Request not found (tk:%s)", toks);
		return;
	}

	debug("Received response (id:%d, tk:%s code:%d)\n", id, toks, received->hdr->code);

	/* check if this is a response to our original request */
	if (!check_token(request, received)) {
		/* drop if this was just some message, or send RST in case of notification */
		if (!sent && (received->hdr->type == COAP_MESSAGE_CON || received->hdr->type == COAP_MESSAGE_NON)) {
			coap_send_rst(ctx, ep, remote, received);
		}
		warn("Token mismatch");
		return;
	}

	if (received->hdr->type == COAP_MESSAGE_RST) {
		warn("Received RST\n");
		return;
	}

	switch (received->hdr->code) {
		/* 2.05 Content - send in response to GET request */
		case COAP_RESPONSE_CODE(205):
			handle_205(client, request, received);
			break;
		default:
			request->response->code = received->hdr->code;
			if (request->cb) {
				request->cb(request, request->response);
				coap_client_delete_request(request);
			}
			break;
	}
}

void coap_client_init(coap_client_t *client, coap_context_t *ctx, coap_endpoint_t *ep)
{
	client->coap = ctx;
	client->ep = ep;
	client->requests = NULL;

	ctx->pdata = client;

	coap_register_option(client->coap, COAP_OPTION_BLOCK2);
	coap_register_response_handler(client->coap, response_handler);
}

void coap_client_deinit(coap_client_t *client)
{
	(void)client;
	//TODO: cancel subscriptions
}

void coap_client_delete_request(coap_request_t *r)
{
	debug("Deleting request [%p]", r);

	if (r->client) {
		HASH_DEL(r->client->requests, r);
	}

	if (r->optlist != NULL) {
		coap_delete_list(r->optlist);
		r->optlist = NULL;
	}

	if (r->data) {
		//mbuf_free((mbuf_t *)r->data);
		coap_free(r->data);
		r->data = NULL;
	}

	if (r->response) {
		if (r->response->data) {
			coap_free(r->response->data);
			//mbuf_free((mbuf_t *)r->response->data);
		}
		coap_free(r->response);
		r->response = NULL;
	}

	coap_cancel_all_messages(r->client->coap, &r->dst, r->token, r->token_length);
	coap_free(r);
}


/**
 * Send given request.
 */
int
coap_client_send_request(
		coap_client_t *client,
		coap_request_t *request,
		coap_response_t *response,
		coap_address_t *dst)
{
	request->client = client;
	request->response = response;

	if (&request->dst != dst) {
		memcpy(&request->dst, dst, sizeof(coap_address_t));
	}

	coap_pdu_t *pdu = create_pdu(request);
	if (!pdu) {
		return 0;
	}

	if (request->block_type) {
		set_blocksize(request);
	}

	// add options if present
	if (request->optlist) {
		coap_list_t *opt;
		/* sort options for delta encoding */
		LL_SORT(request->optlist, order_opts);

		LL_FOREACH(request->optlist, opt) {
			coap_option *o = (coap_option *) (opt->data);
			coap_add_option(pdu, COAP_OPTION_KEY(*o), COAP_OPTION_LENGTH(*o), COAP_OPTION_DATA(*o));
		}
	}

	if (request->data) {
		if (request->block_type > 0) {
			client->block_read(request, pdu, &request->block);
		} else {
			coap_add_data(pdu, request->length, request->data);
		}
	}

	return send_request(request->client, request, request->response, dst, pdu);
}

coap_request_t *coap_client_new_request(unsigned char method, unsigned char type)
{
	coap_request_t *req = coap_malloc(sizeof(coap_request_t));
	if (!req) {
		return NULL;
	}

	memset(req, 0, sizeof(coap_request_t));

	req->method = method;
	req->type = type;
	req->token_length = AUTO_TOKEN_LENGTH;
	prng(req->token, req->token_length);

	return req;
}

static coap_request_t *make_request(
		coap_client_t *client,
		const char *uri,
		unsigned char method,
		unsigned char type,
		void *data,
		size_t len,
		coap_client_cb_t cb)
{
	coap_request_t *req = coap_client_new_request(method, type);
	if (!req)
		return NULL;

	if (!coap_request_set_uri(req, uri, 0)) {
		goto error;
	}

	coap_address(req->uri.host.s, req->uri.port, &req->dst);

	coap_response_t *resp = coap_malloc(sizeof(coap_response_t));
	if (!resp) {
		goto error;
	}
	memset(resp, 0, sizeof(coap_response_t));

	req->data = data;
	req->length = len;
	req->cb = cb;

	if (coap_client_send_request(client, req, resp, &req->dst)) {
		return req;
	}

error:
	if (req) {
		coap_client_delete_request(req);
	}
	return NULL;
}

/**
 *
 * Request a GET operation on a given resource.
 *
 * This function uses CON message type for the request.
 *
 * @param client
 * @param uri
 * @param cb
 *
 */
coap_request_t *coap_client_get(coap_client_t *client, const char *uri, coap_client_cb_t cb)
{
	return make_request(client, uri, COAP_REQUEST_GET, COAP_MESSAGE_CON, NULL, 0, cb);
}

coap_request_t *coap_client_put(coap_client_t *client, const char *uri, void *data, size_t len, coap_client_cb_t cb)
{
	return make_request(client, uri, COAP_REQUEST_PUT, COAP_MESSAGE_CON, data, len, cb);
}

coap_request_t *coap_client_post(coap_client_t *client, const char *uri, void *data, size_t len, coap_client_cb_t cb)
{
	return make_request(client, uri, COAP_REQUEST_POST, COAP_MESSAGE_CON, data, len, cb);
}

/*
response_t *coap_client_subscribe(coap_client_t *client, const char *uri, notification_cb_t cb)
{
	return NULL;
}

response_t *coap_client_unsubscribe(coap_client_t *client, const char *uri)
{
	return NULL;
}
*/
