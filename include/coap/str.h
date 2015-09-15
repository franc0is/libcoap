/* str.h -- strings to be used in the CoAP library
 *
 * Copyright (C) 2010,2011 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#ifndef _COAP_STR_H_
#define _COAP_STR_H_

#include <string.h>

#ifndef MIN
#define MIN(x , y)  (((x) < (y)) ? (x) : (y))
#endif

typedef struct {
  size_t length;    /* length of string */
  unsigned char *s; /* string data */
} str;

#define COAP_SET_STR(st,l,v) { (st)->length = (l), (st)->s = (v); }

/**
 * Returns a new string object with at least size bytes storage allocated. The
 * string must be released using coap_delete_string();
 */
str *coap_new_string(size_t size);

/**
 * Deletes the given string and releases any memory allocated.
 */
void coap_delete_string(str *);

/**
 * Compare CoAP string with a null terminated string.
 */
static inline int coap_strcmp(const str *s1, const char *s2)
{
	return memcmp(s1->s, s2, MIN(s1->length, strlen(s2) - 1));
}

#endif /* _COAP_STR_H_ */
