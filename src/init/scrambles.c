
#include "scrambles.h"

char read_char_static( char x_in, char y_in ) {
   return ~((x_in & y_in) | (~x_in & ~y_in));
}

void descramble_copy_string( char* dest_in, char* string_in, char* key_in ) {
   char c;
   int i = 0;

   while( '\0' != (c = string_in[i]) ) {

      dest_in[i] = read_char_static( string_in[i], key_in[i] );

      /* Iterate. */
      i++;
   }
}

/* Purpose: Create a dynamic descrambled copy of the given scrambled string.  */
/* Return: A pointer to the new string. Must be freed manually.               */
char* descramble_create_string( char* string_in, char* key_in ) {
   char* pc_out;

   pc_out = calloc( strlen( string_in ), sizeof( char ) );
   descramble_copy_string( pc_out, string_in, key_in );

   return pc_out;
}

