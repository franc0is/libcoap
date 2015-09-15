  /* debug.h -- debug utilities
 *
 * Copyright (C) 2010,2011,2014 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see
 * README for terms of use.
 */

#ifndef _COAP_DEBUG_H_
#define _COAP_DEBUG_H_

#ifndef COAP_DEBUG_FD
#define COAP_DEBUG_FD stdout
#endif

#ifndef COAP_ERR_FD
#define COAP_ERR_FD stderr
#endif

#include <inttypes.h>

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
typedef short coap_log_t;
#else
/** Pre-defined log levels akin to what is used in \b syslog. */
typedef enum {
  LOG_EMERG=0,
  LOG_ALERT,
  LOG_CRIT,
  LOG_WARNING,
  LOG_NOTICE,
  LOG_INFO,
  LOG_DEBUG
} coap_log_t;
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#include <stdio.h>
/** Returns a textual description of the message type @p t. */
static inline const char *
msg_type_string(uint8_t t) {
  static const char * const types[] = { "CON", "NON", "ACK", "RST", "???" };

  return types[min(t, sizeof(types)/sizeof(char *) - 1)];
}

/** Returns a textual description of the method or response code. */
static inline const char *
msg_code_string(uint8_t c) {
  static const char * const methods[] = { "0.00", "GET", "POST", "PUT", "DELETE", "PATCH" };
  static char buf[5];

  if (c < sizeof(methods)/sizeof(char *)) {
    return methods[c];
  } else {
    snprintf(buf, sizeof(buf), "%u.%02u", c >> 5, c & 0x1f);
    return buf;
  }
}

/** Returns the current log level. */
coap_log_t coap_get_log_level(void);

/** Sets the log level to the specified value. */
void coap_set_log_level(coap_log_t level);

/** Returns a zero-terminated string with the name of this library. */
const char *coap_package_name(void);

/** Returns a zero-terminated string with the library version. */
const char *coap_package_version(void);

/**
 * Writes the given text to @c COAP_ERR_FD (for @p level <= @c LOG_CRIT) or @c
 * COAP_DEBUG_FD (for @p level >= @c LOG_WARNING). The text is output only when
 * @p level is below or equal to the log level that set by coap_set_log_level().
 */
void coap_log_impl(const char *file, int line, coap_log_t level, const char *format, ...);

#ifndef coap_log
#define coap_log(...) coap_log_impl(__FILE__, __LINE__, __VA_ARGS__)
#endif

#ifndef NDEBUG

/* A set of convenience macros for common log levels. */
#ifndef info
#define info(...) coap_log(LOG_INFO, __VA_ARGS__)
#endif

#ifndef warn
#define warn(...) coap_log(LOG_WARNING, __VA_ARGS__)
#endif

#ifndef debug
#define debug(...) coap_log(LOG_DEBUG, __VA_ARGS__)
#endif

#define critical(...) coap_log(LOG_CRIT, __VA_ARGS__)

#include "pdu.h"
void coap_show_pdu(const coap_pdu_t *);

struct coap_address_t;
size_t coap_print_addr(const struct coap_address_t *, unsigned char *, size_t);

#else

#define debug(...)
#define info(...)
#define warn(...)

#define coap_show_pdu(x)
#define coap_print_addr(...)

#endif

#endif /* _COAP_DEBUG_H_ */
