#ifndef _PLATFORM_IO_H_
#define _PLATFORM_IO_H_

#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <errno.h>

struct coap_packet_t {
  coap_if_handle_t hnd;     /**< the interface handle */
  coap_address_t src;       /**< the packet's source address */
  coap_address_t dst;       /**< the packet's destination address */
  const coap_endpoint_t *interface;

  int ifindex;
  void *session;            /**< opaque session data */

  size_t length;            /**< length of payload */
  unsigned char payload[];  /**< payload */
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
