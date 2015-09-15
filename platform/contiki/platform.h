/*
 * coap_contiki.h
 *
 *  Created on: Jun 11, 2015
 *      Author: wojtek
 */

#ifndef INCLUDE_COAP_COAP_CONTIKI_H_
#define INCLUDE_COAP_COAP_CONTIKI_H_

#include <string.h>
#include "clock.h"
#include "mem.h"
#include "net/ip/uip-debug.h"

typedef clock_time_t coap_tick_t;
typedef clock_time_t coap_time_t;

/**
 * This data type is used to represent the difference between two clock_tick_t
 * values. This data type must have the same size in memory as coap_tick_t to
 * allow wrapping.
 */
typedef int coap_tick_diff_t;

#define COAP_TICKS_PER_SECOND CLOCK_SECOND

static inline void coap_clock_init(void) {
  clock_init();
}

static inline void coap_ticks(coap_tick_t *t) {
  *t = clock_time();
}

static inline coap_time_t coap_ticks_to_rt(coap_tick_t t) {
  return t / COAP_TICKS_PER_SECOND;
}

/**
 * Fills \p buf with \p len random bytes. This is the default implementation for
 * prng(). You might want to change prng() to use a better PRNG on your specific
 * platform.
 */
static inline int
coap_prng_impl(unsigned char *buf, size_t len) {
  unsigned short v = random_rand();
  while (len > sizeof(v)) {
    memcpy(buf, &v, sizeof(v));
    len -= sizeof(v);
    buf += sizeof(v);
    v = random_rand();
  }

  memcpy(buf, &v, len);
  return 1;
}

#define prng(Buf,Length) coap_prng_impl((Buf), (Length))
#define prng_init(Value) random_init((unsigned short)(Value))

#endif /* INCLUDE_COAP_COAP_CONTIKI_H_ */
