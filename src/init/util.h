
#ifndef UTIL_H
#define UTIL_H

/* Needed for vasprintf(). */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/* #include <varargs.h> */
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <regex.h>

#include "error.h"

/* = Function Prototypes = */

char* xasprintf( const char*, ... );
char* last_char_is( const char *, int );
int fork_exec( char** );
int kill_pid_file( char* );
int parse_cmd_line( void );

#endif /* UTIL_H */

