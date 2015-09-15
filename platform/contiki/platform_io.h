#ifndef _PLATFORM_IO_H_
#define _PLATFORM_IO_H_

#include "uip.h"

#define CUSTOM_CONTEXT_FIELDS \
  struct uip_udp_conn *conn;      \
  struct etimer retransmit_timer; \
  struct etimer notify_timer;

/**< uIP connection object */
/**< fires when the next packet must be sent */
/**< used to check resources periodically */

typedef struct coap_address_t {
  uip_ipaddr_t addr;
  unsigned short port;
} coap_address_t;

#define _coap_address_equals_impl(A,B) \
        ((A)->port == (B)->port        \
        && uip_ipaddr_cmp(&((A)->addr),&((B)->addr)))

/** @todo implementation of _coap_address_isany_impl() for Contiki */
#define _coap_address_isany_impl(A)  0

#define _coap_is_mcast_impl(Address) uip_is_addr_mcast(&((Address)->addr))

/**
 * Abstraction of virtual endpoint that can be attached to coap_context_t. The
 * tuple (handle, addr) must uniquely identify this endpoint.
 */
typedef struct coap_endpoint_t {
  coap_network_read_t network_read;
  coap_network_send_t network_send;
  void *conn;   /**< opaque connection (e.g. uip_conn in Contiki) */
  coap_address_t addr; /**< local interface address */
  int ifindex;
  int flags;
} coap_endpoint_t;

/*
 * This is only included in coap_io.h instead of .c in order to be available for
 * sizeof in mem.c.
 */
struct coap_packet_t {
  coap_if_handle_t hnd;         /**< the interface handle */
  coap_address_t src;           /**< the packet's source address */
  coap_address_t dst;           /**< the packet's destination address */
  const coap_endpoint_t *interface;
  int ifindex;
  void *session;                /**< opaque session data */
  size_t length;                /**< length of payload */
  unsigned char payload[];      /**< payload */
};

#endif /* _PLATFORM_IO_H_ */
