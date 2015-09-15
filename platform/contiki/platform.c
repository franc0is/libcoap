/*
 * coap_contiki.c

 *
 *  Created on: Jun 11, 2015
 *      Author: wojtek
 */
# ifndef DEBUG
#  define DEBUG DEBUG_PRINT
# endif /* DEBUG */

#include "coap_config.h"
#include "net.h"
#include "pdu.h"
#include "coap_io.h"
#include "resource.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[UIP_LLIPH_LEN])

#define COAP_MAX_STRING_SIZE 12
#define COAP_MAX_STRINGS      8
#define COAP_MAX_PACKET_SIZE (sizeof(coap_packet_t) + COAP_MAX_PDU_SIZE)
#define COAP_MAX_PACKETS     2

static unsigned char initialized = 0;
static coap_context_t the_coap_context;

PROCESS(coap_retransmit_process, "message retransmit process");

struct coap_string_t
{
  char data[COAP_MAX_STRING_SIZE];
};

typedef union
{
  coap_pdu_t packet; /* try to convince the compiler to word-align this structure  */
  char buf[COAP_MAX_PACKET_SIZE];
} coap_packetbuf_t;

MEMB(packet_storage, coap_packetbuf_t, COAP_MAX_PACKETS);
MEMB(node_storage, coap_queue_t, COAP_PDU_MAXCNT);
MEMB(pdu_storage, coap_pdu_t, COAP_PDU_MAXCNT);
MEMB(pdu_buf_storage, coap_packetbuf_t, COAP_PDU_MAXCNT);
MEMB(resource_storage, coap_resource_t, COAP_MAX_RESOURCES);
MEMB(attribute_storage, coap_attr_t, COAP_MAX_ATTRIBUTES);
MEMB(subscription_storage, coap_subscription_t, COAP_MAX_SUBSCRIBERS);
MEMB(string_storage, struct coap_string_t, COAP_MAX_STRINGS);

static struct memb *
get_container(coap_memory_tag_t type)
{
  switch (type) {
    case COAP_PACKET:
      return &packet_storage;
    case COAP_NODE:
      return &node_storage;
    case COAP_PDU:
      return &pdu_storage;
    case COAP_PDU_BUF:
      return &pdu_buf_storage;
    case COAP_RESOURCE:
      return &resource_storage;
    case COAP_RESOURCEATTR:
      return &attribute_storage;
    case COAP_SUBSCRIPTION:
      return &subscription_storage;
    default:
      return &string_storage;
  }
}

void
coap_memory_init(void)
{
  memb_init(&string_storage);
  memb_init(&packet_storage);
  memb_init(&node_storage);
  memb_init(&pdu_storage);
  memb_init(&pdu_buf_storage);
  memb_init(&resource_storage);
  memb_init(&attribute_storage);
}

void *
coap_malloc_type(coap_memory_tag_t type, size_t size)
{
  if (type != COAP_CONTEXT) {
    struct memb *container = get_container(type);

    assert(container);

    if (size > container->size) {
      debug("coap_malloc_type: Requested memory exceeds maximum object size\n");
      return NULL;
    }

    return memb_alloc(container);
  } else if (!initialized) {
    return &the_coap_context;
  } else {
    return NULL;
  }
}

void
coap_free_type(coap_memory_tag_t type, void *object)
{
  if (type != COAP_CONTEXT)
    memb_free(get_container(type), object);
}

void
coap_platform_init(void)
{
  coap_resources_init();
  coap_memory_init();

  process_start(&coap_retransmit_process, (char *) c);

  PROCESS_CONTEXT_BEGIN(&coap_retransmit_process);
#ifndef WITHOUT_OBSERVE
  etimer_set(&c->notify_timer,
             COAP_RESOURCE_CHECK_TIME * COAP_TICKS_PER_SECOND);
#endif /* WITHOUT_OBSERVE */
  /* the retransmit timer must be initialized to some large value */
  etimer_set(&the_coap_context.retransmit_timer, 0xFFFF);
  PROCESS_CONTEXT_END(&coap_retransmit_process);

  c = &the_coap_context;
  initialized = 1;
}

void
coap_platform_deinit(void)
{
  memset(&the_coap_context, 0, sizeof(coap_context_t));
  initialized = 0;
}

/*---------------------------------------------------------------------------*/
/* CoAP message retransmission */
/*---------------------------------------------------------------------------*/
PROCESS_THREAD( coap_retransmit_process, ev, data)
{
  coap_tick_t now;
  coap_queue_t *nextpdu;

  PROCESS_BEGIN();

  debug("Started retransmit process\r\n");

  while (1) {
    PROCESS_YIELD();
    if (ev == PROCESS_EVENT_TIMER) {
      if (etimer_expired(&the_coap_context.retransmit_timer)) {

        nextpdu = coap_peek_next(&the_coap_context);

        coap_ticks(&now);
        while (nextpdu && nextpdu->t <= now) {
          coap_retransmit(&the_coap_context, coap_pop_next(&the_coap_context));
          nextpdu = coap_peek_next(&the_coap_context);
        }

        /* need to set timer to some value even if no nextpdu is available */
        etimer_set(&the_coap_context.retransmit_timer,
                   nextpdu ? nextpdu->t - now : 0xFFFF);
      }
#ifndef WITHOUT_OBSERVE
      if (etimer_expired(&the_coap_context.notify_timer)) {
        coap_check_notify(&the_coap_context);
        etimer_reset(&the_coap_context.notify_timer);
      }
#endif /* WITHOUT_OBSERVE */
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
