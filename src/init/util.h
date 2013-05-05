
#ifndef UTIL_H
#define UTIL_H

/* Needed for vasprintf(). */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* #include <varargs.h> */

/* = Function Prototypes = */

char* xasprintf( const char*, ... );
char* last_char_is( const char *, int );

#endif /* UTIL_H */

