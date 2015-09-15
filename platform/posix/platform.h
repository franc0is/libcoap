/*
 * coap_posix.h
 *
 *  Created on: Jun 11, 2015
 *      Author: wojtek
 */

#ifndef INCLUDE_COAP_COAP_POSIX_H_
#define INCLUDE_COAP_COAP_POSIX_H_

#include <time.h>

/**
 * This data type represents internal timer ticks with COAP_TICKS_PER_SECOND
 * resolution.
 */
typedef unsigned long coap_tick_t;

/**
 * CoAP time in seconds since epoch.
 */
typedef time_t coap_time_t;

/**
 * This data type is used to represent the difference between two clock_tick_t
 * values. This data type must have the same size in memory as coap_tick_t to
 * allow wrapping.
 */
typedef long coap_tick_diff_t;

/** Use ms resolution on POSIX systems */
#define COAP_TICKS_PER_SECOND 1000

/**
 * Sets @p t to the internal time with COAP_TICKS_PER_SECOND resolution.
 */
static inline void coap_ticks(coap_tick_t *t) {
  struct timespec spec;
  clock_gettime(CLOCK_MONOTONIC, &spec);
  *t = (coap_tick_t)spec.tv_sec*1000 + (coap_tick_t)(spec.tv_nsec / 1.0e6);
}

/**
 * Helper function that converts coap ticks to wallclock time. On POSIX, this
 * function returns the number of seconds since the epoch. On other systems, it
 * may be the calculated number of seconds since last reboot or so.
 *
 * @param t Internal system ticks.
 *
 * @return  The number of seconds that has passed since a specific reference
 *          point (seconds since epoch on POSIX).
 */
static inline coap_time_t coap_ticks_to_rt(coap_tick_t t) {
  return t / COAP_TICKS_PER_SECOND;
}

#endif /* INCLUDE_COAP_COAP_POSIX_H_ */
