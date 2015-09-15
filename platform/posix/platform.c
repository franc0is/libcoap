/*
 * coap_posix.c
 *
 *  Created on: Jun 11, 2015
 *      Author: wojtek
 */

#include <stddef.h>
#include <coap/mem.h>
#include <coap/prng.h>

#ifdef HAVE_ASSERT_H
#include <assert.h>
#else /* HAVE_ASSERT_H */
#define assert(...)
#endif /* HAVE_ASSERT_H */

#ifdef HAVE_MALLOC
#include <stdlib.h>
#endif

#ifdef __GNUC__
#define UNUSED_PARAM __attribute__((unused))
#else
#define UNUSED_PARAM
#endif /* __GNUC__ */

void
coap_platform_init(void) {
  prng_init(time(NULL));
}

void
coap_memory_init(void) {
}

void *
coap_malloc_type(coap_memory_tag_t type UNUSED_PARAM, size_t size) {
  return malloc(size);
}

void
coap_free_type(coap_memory_tag_t type UNUSED_PARAM, void *p) {
  free(p);
}
