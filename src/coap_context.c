#include "coap_config.h"

#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#elif HAVE_SYS_UNISTD_H
#include <sys/unistd.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "debug.h"
#include "mem.h"
#include "str.h"
#include "async.h"
#include "resource.h"
#include "option.h"
#include "encode.h"
#include "block.h"
#include "net.h"
#include "coap_timer.h"
#include "coap_io.h"

#ifndef WITHOUT_OBSERVE
static void notify_timer_cb(void *data) {
  coap_context_t *c = data;
  coap_check_notify(c);
  coap_timer_set(c->notify_timer, COAP_RESOURCE_CHECK_TIME * COAP_TICKS_PER_SECOND);
}
#endif

// TODO this should probably be in its own file ... coap_retransmit.c?
static void retransmit_timer_cb(void *data) {
  coap_context_t *c = data;
  coap_queue_t *nextpdu = coap_peek_next(c);

  coap_tick_t now;
  coap_ticks(&now);

  while (nextpdu && nextpdu->t <= now) {
    coap_retransmit(c, coap_pop_next(c));
    nextpdu = coap_peek_next(c);
  }

  if (nextpdu) {
    coap_timer_set(c->retransmit_timer, nextpdu->t - now);
  }
}

coap_context_t*
coap_new_context(void) {

  //FIXME: move this to coap_platform_init()?
  coap_timer_init();
  coap_clock_init();
  coap_tick_t now;
  coap_ticks(&now);
  prng_init((ptrdiff_t)now);

  coap_context_t *c = coap_malloc_type(COAP_CONTEXT, sizeof( coap_context_t ) );
  if (!c) {
    coap_log(LOG_EMERG, "coap_init: malloc:\n");
    return NULL;
  }
  memset(c, 0, sizeof( coap_context_t ) );

  /* initialize message id */
  prng((unsigned char *)&c->message_id, sizeof(unsigned short));

  /* register the critical options that we know */
  coap_register_option(c, COAP_OPTION_IF_MATCH);
  coap_register_option(c, COAP_OPTION_URI_HOST);
  coap_register_option(c, COAP_OPTION_IF_NONE_MATCH);
  coap_register_option(c, COAP_OPTION_URI_PORT);
  coap_register_option(c, COAP_OPTION_URI_PATH);
  coap_register_option(c, COAP_OPTION_URI_QUERY);
  coap_register_option(c, COAP_OPTION_ACCEPT);
  coap_register_option(c, COAP_OPTION_PROXY_URI);
  coap_register_option(c, COAP_OPTION_PROXY_SCHEME);
  coap_register_option(c, COAP_OPTION_BLOCK2);
  coap_register_option(c, COAP_OPTION_BLOCK1);

#if 0
  c->endpoint = coap_new_endpoint(listen_addr, COAP_ENDPOINT_NOSEC);
  if (c->endpoint == NULL) {
    goto onerror;
  }
#endif

# ifndef WITHOUT_OBSERVE
  c->notify_timer = coap_new_timer(notify_timer_cb, (void *)c);
  coap_timer_set(c->notify_timer, COAP_RESOURCE_CHECK_TIME * COAP_TICKS_PER_SECOND);
#endif /* WITHOUT_OBSERVE */

  c->retransmit_timer = coap_new_timer(retransmit_timer_cb, (void *)c);

  return c;

 onerror:
  coap_free(c);
  return NULL;
}

void
coap_free_context(coap_context_t *context) {

  if (!context)
    return;

  coap_delete_all(context->sendqueue);

  coap_free_timer(context->retransmit_timer);
#ifndef WITHOUT_OBSERVE
  coap_free_timer(context->notify_timer);
#endif

  coap_delete_all_resources(context);
  coap_free_type(COAP_CONTEXT, context);
}

