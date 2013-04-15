
#include "datatypes.h"

char read_char_static( char x_in, char y_in ) {
   return ~((x_in & y_in) | (~x_in & ~y_in));
}

void decode_copy_string( char* dest_in, char* string_in, char* key_in ) {
   char c;
   int i = 0;

   while( '\0' != (c = string_in[i]) ) {

      dest_in[i] = read_char_static( string_in[i], key_in[i] );

      /* Iterate. */
      i++;
   }
}

