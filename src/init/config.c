
#include "config.h"

#include "config_base.h"

/* = Configuration Helpers = */

/* Notes:   This function is for internal use within this module only.        *
 *          Properties should be accessed through a specialized configuration *
 *          loader function defined below.                                    */
static char config_descramble_char( char x_in, char y_in ) {
   return ~((x_in & y_in) | (~x_in & ~y_in));
}

/* Purpose: Take a hard-coded scrambled string and return a dynamically-      *
 *          allocated descrambled string pointer.                             */
char* config_descramble_string( const char* pc_string_in, const int i_len_in ) {
   char* pc_out;
   int i;

   /* +1 for the null terminator. */
   pc_out = calloc( i_len_in + 1, sizeof( char ) );

   for( i = 0 ; i < i_len_in ; i++ ) {
      pc_out[i] = config_descramble_char( pc_string_in[i], gac_skey[i] );
   }

   return pc_out;
}

/* Notes:   The string passed as pc_string_in should be free()ed afterwards.  */
char** config_split_string_array( const char* pc_string_in ) {
   char** ppc_out = NULL,
      c;
   int i = 0, j = 0,
      i_string_count = 0;

   /* At the very least, we'll end up with an empty array (just a NULL). */
   ppc_out = calloc( 1, sizeof( char* ) );

   while( '\0' != (c = pc_string_in[i]) ) {

      if( '|' == c || 0 == i_string_count ) {
         /* Create a new string in the array (and leave room for the NULL.    *
          * terminator.                                                       */
         i_string_count++;
         ppc_out = realloc( ppc_out, (i_string_count + 1) * sizeof( char* ) );
         ppc_out[i_string_count - 1] = calloc( 1, sizeof( char ) );
         j = 0;

         if( '|' == c ) {
            /* Grab the character after the separator. */
            c = pc_string_in[++i];
         }
      }

      /* Add the grabbed character to the new string. */
      ppc_out[i_string_count - 1] = realloc(
         ppc_out[i_string_count - 1],
         (j + 2) * sizeof( char )
      );
      ppc_out[i_string_count - 1][j] = c;
      ppc_out[i_string_count - 1][j + 1] = '\0';

      /* Increment. */
      i++;
      j++;
   }

   return ppc_out;
}

void config_free_string_array( char** ppc_string_array_in ) {
   int i = 0;

   while( NULL != ppc_string_array_in[i] ) {
      free( ppc_string_array_in[i] );
      i++;
   }
   free( ppc_string_array_in );
}

/* = Configuration Loaders = */

struct string_holder* config_split_string_holders(
   const char* pc_string_holders_in
) {
   struct string_holder* ps_string_holders = NULL,
      * ps_string_holder_iter = NULL,
      * ps_string_holder_prev = NULL;
   char* pc_current_field = NULL;
   int i = 0, j = 0;

   /* `md_arrays',`md1</dev/sda1|/dev/sdb1>md2</dev/sda2|/dev/sdb2>' */

   /* Parse out the arrays and create structs for them. */
   while( '\0' != pc_string_holders_in[i] ) {
      if( NULL == ps_string_holder_iter && NULL == ps_string_holders ) {
         /* Create the first MD array in the list. */
         ps_string_holders = calloc( 1, sizeof( struct string_holder ) );
         ps_string_holders->name = calloc( 1, sizeof( char ) );
         ps_string_holder_iter = ps_string_holders;
         pc_current_field = ps_string_holder_iter->name;
         j = 0;
      }

      if( '<' == pc_string_holders_in[i] ) {
         /* Begin collecting a string to split for the dev list. */
         pc_current_field = calloc( 1, sizeof( char ) );
         j = 0;
         i++;
      }

      if( '>' == pc_string_holders_in[i] ) {
         /* Convert the gathered string into an array of dev names. */
         ps_string_holder_iter->strings =
            config_split_string_array( pc_current_field );
         free( pc_current_field );

         if(
            '\0' != pc_string_holders_in[i + 1] &&
            10 != pc_string_holders_in[i + 1] /* Newline/linefeed. */
         ) {
            /* Add a new MD array to the list and iterate. */
            ps_string_holder_prev = ps_string_holder_iter;
            ps_string_holder_iter->next =
               calloc( 1, sizeof( struct string_holder ) );
            ps_string_holder_iter = ps_string_holder_iter->next;
            ps_string_holder_iter->next = NULL;
            ps_string_holder_iter->name = calloc( 1, sizeof( char ) );
            pc_current_field = ps_string_holder_iter->name;

            /* Move on to the next array or the end of the string. */
            j = 0;
            i++;
            continue;
         } else {
            break;
         }
      }

      /* Add the grabbed character to the current string. */
      pc_current_field = realloc( pc_current_field, (j + 2) * sizeof( char ) );
      pc_current_field[j] = pc_string_holders_in[i];
      pc_current_field[j + 1] = '\0';

      /* Increment. */
      i++;
      j++;
   }

   return ps_string_holders;
}

void config_free_string_holders( struct string_holder* ps_string_holders_in ) {
   struct string_holder* ps_string_holder_iter = ps_string_holders_in,
      * ps_string_holder_prev = NULL;

   while( NULL != ps_string_holder_iter ) {
      ps_string_holder_prev = ps_string_holder_iter;
      ps_string_holder_iter = ps_string_holder_iter->next;
      free( ps_string_holder_prev );
   }
}

