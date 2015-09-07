#ifndef _PLATFORM_IO_H_
#define _PLATFORM_IO_H_

#include <lwip/udp.h>

/*
 * This is only included in coap_io.h instead of .c in order to be available for
 * sizeof in lwippools.h.
 * Simple carry-over of the incoming pbuf that is later turned into a node.
 *
 * Source address data is currently side-banded via ip_current_dest_addr & co
 * as the packets have limited lifetime anyway.
 */
struct coap_packet_t {
  unsigned char *data;
  size_t data_len;
  const coap_endpoint_t *local_interface;
  uint16_t srcport;
};

typedef struct coap_endpoint_t {
  struct udp_pcb *pcb;

  coap_address_t addr; /**< local interface address */
  int ifindex;
  int flags;
} coap_endpoint_t;

/**
 * Get the pbuf of a packet. The caller takes over responsibility for freeing
 * the pbuf.
 */
struct pbuf *coap_packet_extract_pbuf(coap_packet_t *packet);


#endif /* _PLATFORM_IO_H_ */
