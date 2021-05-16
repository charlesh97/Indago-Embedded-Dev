#ifndef _JANSSON_PRIVATE_CONFIG_H
#define _JANSSON_PRIVATE_CONFIG_H

#include <stdint.h>


/* If your compiler supports the inline keyword in C, JSON_INLINE is
   defined to `inline', otherwise empty. In C++, the inline is always
   supported. */
#ifdef __cplusplus
#define JSON_INLINE inline
#else
#define JSON_INLINE inline
#endif

/* If your compiler supports the `long long` type and the strtoll()
   library function, JSON_INTEGER_IS_LONG_LONG is defined to 1,
   otherwise to 0. */
#define JSON_INTEGER_IS_LONG_LONG 1

/* If locale.h and localeconv() are available, define to 1,
   otherwise to 0. */
#define JSON_HAVE_LOCALECONV 1

/* If __atomic builtins are available they will be used to manage
   reference counts of json_t. */
#define JSON_HAVE_ATOMIC_BUILTINS 0

/* If __atomic builtins are not available we try using __sync builtins
   to manage reference counts of json_t. */
#define JSON_HAVE_SYNC_BUILTINS 0

/* Maximum recursion depth for parsing JSON input.
   This limits the depth of e.g. array-within-array constructions. */
#define JSON_PARSER_MAX_DEPTH 2048


#define HAVE_CONFIG_H 1

#define HAVE_STDINT_H 1


#endif