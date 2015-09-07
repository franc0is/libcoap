#ifndef _PLATFORM_IO_H_
#define _PLATFORM_IO_H_

#include "uip.h"

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

/**
 * Abstraction of virtual endpoint that can be attached to coap_context_t. The
 * tuple (handle, addr) must uniquely identify this endpoint.
 */
typedef struct coap_endpoint_t {
  union {
    int fd;       /**< on POSIX systems */
    void *conn;   /**< opaque connection (e.g. uip_conn in Contiki) */
  } handle;       /**< opaque handle to identify this endpoint */

  coap_address_t addr; /**< local interface address */
  int ifindex;
  int flags;
} coap_endpoint_t;

#endif /* _PLATFORM_IO_H_ */
