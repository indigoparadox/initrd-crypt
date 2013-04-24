
#include "util.h"

/* TODO: Why do we need to include this twice? */
#include <stdarg.h>

/* Portions of this code shamelessly stolen from busybox (but changed to      *
 * fit our preferred coding style.                                            */

char* xasprintf( const char* pc_format_in, ... ) {
   va_list p;
   int i_retval;
   char *pc_string;

   va_start( p, pc_format_in );
   i_retval = vasprintf( &pc_string, pc_format_in, p );
   va_end( p );

   if( 0 > i_retval ) {
      return NULL;
   } else {
      return pc_string;
   }
}

char* last_char_is( const char *s, int c ) {
   if( s && *s ) {
      size_t sz = strlen(s) - 1;
      s += sz;
      if( (unsigned char)*s == c ) {
         return (char*)s;
      }
   }

   return NULL;
}

