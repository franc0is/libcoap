/*
 * coap_lwip.h
 *
 *  Created on: Jun 11, 2015
 *      Author: wojtek
 */

#ifndef INCLUDE_COAP_COAP_LWIP_H_
#define INCLUDE_COAP_COAP_LWIP_H_

#include <stdint.h>
#include <lwip/sys.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/timers.h>
#include <lwip/memp.h>

/* lwIP provides ms in sys_now */
#define COAP_TICKS_PER_SECOND 1000

typedef uint32_t coap_tick_t;
typedef uint32_t coap_time_t;
typedef int32_t coap_tick_diff_t;

static inline void coap_ticks_impl(coap_tick_t *t) {
  *t = sys_now();
}

static inline void coap_clock_init_impl(void) {
}

#define coap_clock_init coap_clock_init_impl
#define coap_ticks coap_ticks_impl

static inline coap_time_t coap_ticks_to_rt(coap_tick_t t) {
  return t / COAP_TICKS_PER_SECOND;
}

static void coap_retransmittimer_execute(void *arg);
static void coap_retransmittimer_restart(coap_context_t *ctx);

/* no initialization needed with lwip (or, more precisely: lwip must be
 * completely initialized anyway by the time coap gets active)  */
static inline void coap_memory_init(void) {}

/* It would be nice to check that size equals the size given at the memp
 * declaration, but i currently don't see a standard way to check that without
 * sourcing the custom memp pools and becoming dependent of its syntax
 */
#define coap_malloc_type(type, size) memp_malloc(MEMP_##type)
#define coap_free_type(type, p) memp_free(MEMP_##type, p)

/* Those are just here to make uri.c happy where string allocation has not been
 * made conditional.
 */
static inline void *coap_malloc(size_t size) {
  LWIP_ASSERT("coap_malloc must not be used in lwIP", 0);
}

static inline void coap_free(void *pointer) {
  LWIP_ASSERT("coap_free must not be used in lwIP", 0);
}

#endif /* INCLUDE_COAP_COAP_LWIP_H_ */
