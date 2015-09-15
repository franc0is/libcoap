/* coap_io.h -- Default network I/O functions for libcoap
 *
 * Copyright (C) 2012--2013 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#ifndef _COAP_IO_H_
#define _COAP_IO_H_

#ifdef HAVE_ASSERT_H
#include <assert.h>
#else
#ifndef assert
#warning "assertions are disabled"
#  define assert(x)
#endif
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

/** Invalid interface handle */
#define COAP_IF_INVALID -1

#define COAP_ENDPOINT_NOSEC 0x00
#define COAP_ENDPOINT_DTLS  0x01

#include "pdu.h"
#include "platform_io.h"

struct coap_context_t;
typedef struct coap_context_t coap_context_t;

struct coap_packet_t;
typedef struct coap_packet_t coap_packet_t;

struct coap_endpoint_t;
typedef struct coap_endpoint_t coap_endpoint_t;

coap_endpoint_t *coap_new_endpoint(const coap_address_t *addr, int flags);

void coap_free_endpoint(coap_endpoint_t *ep);

/**
 * Function interface for data transmission. This function returns the number of
 * bytes that have been transmitted, or a value less than zero on error.
 *
 * @param context          The calling CoAP context.
 * @param local_interface  The local interface to send the data.
 * @param dst              The address of the receiver.
 * @param pdu              The pdu which should be send.
 *
 * @return                 The number of bytes written on success, or a value
 *                         less than zero on error.
 */
typedef ssize_t (*coap_network_send_t)(coap_context_t *context,
                          const coap_endpoint_t *local_interface,
                          const coap_address_t *dst, const coap_pdu_t *pdu);

/**
 * Function interface for reading data. This function returns the number of
 * bytes that have been read, or a value less than zero on error. In case of an
 * error, @p *packet is set to NULL.
 *
 * @param ep     The endpoint that is used for reading data from the network.
 * @param packet A result parameter where a pointer to the received packet
 *               structure is stored. The caller must call coap_free_packet to
 *               release the storage used by this packet.
 *
 * @return       The number of bytes received on success, or a value less than
 *               zero on error.
 */
typedef ssize_t (*coap_network_read_t)(coap_endpoint_t *ep, coap_packet_t **packet);

/**
 * Abstraction of virtual endpoint that can be attached to coap_context_t. The
 * tuple (handle, addr) must uniquely identify this endpoint.
 */
struct coap_endpoint_t {
  coap_address_t addr; /**< local interface address */
  int ifindex;
  int flags;
  coap_network_read_t network_read;
  coap_network_send_t network_send;
  PLATFORM_ENDPOINT_PROPERTIES
};

struct coap_packet_t {
  coap_address_t src;       /**< the packet's source address */
  coap_address_t dst;       /**< the packet's destination address */
  const coap_endpoint_t *interface;

  int ifindex;
  void *session;            /**< opaque session data */

  size_t length;            /**< length of payload */
  unsigned char payload[];  /**< payload */
};

#ifndef coap_mcast_interface
# define coap_mcast_interface(Local) 0
#endif

/** Allocates store for a packet */
coap_packet_t *coap_malloc_packet(void);

/**
 * Releases the storage allocated for @p packet.
 */
void coap_free_packet(coap_packet_t *packet);

/**
 * Populate the coap_endpoint_t *target from the incoming packet's destination
 * data.
 *
 * This is usually used to copy a packet's data into a node's local_if member.
 */
void coap_packet_populate_endpoint(coap_packet_t *packet,
                                   coap_endpoint_t *target);

/**
 * Given an incoming packet, copy its source address into an address struct.
 */
void coap_packet_copy_source(coap_packet_t *packet, coap_address_t *target);

/**
 * Given a packet, set msg and msg_len to an address and length of the packet's
 * data in memory.
 * */
void coap_packet_get_memmapped(coap_packet_t *packet,
                               unsigned char **address,
                               size_t *length);


size_t
coap_get_max_packetlength(const coap_packet_t *packet);

/**
 * Checks if a message with destination address @p dst matches the
 * local interface with address @p local. This function returns @c 1
 * if @p dst is a valid match, and @c 0 otherwise.
 */
int
coap_is_local_if(const coap_address_t *local, const coap_address_t *dst);

#endif /* _COAP_IO_H_ */
