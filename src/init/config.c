
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

/* = Configuration Loaders = */

MD_ARRAY* config_load_md_arrays( void ) {
   MD_ARRAY* ps_md_arrays = NULL,
      * ps_md_array_iter = NULL,
      * ps_md_array_prev = NULL;
   /* TODO: How should we decide the max potential matches? */
   char* pc_md_arrays = NULL,
      * pc_current_field = NULL;
   int i = 0, j = 0;

   /* `md_arrays',`md1</dev/sda1|/dev/sdb1>md2</dev/sda2|/dev/sdb2>' */

   pc_md_arrays = config_descramble_string( gac_md_arrays, gai_md_arrays );

   /* Parse out the arrays and create structs for them. */
   while( '\0' != pc_md_arrays[i] ) {
      //if( NULL == pc_current_field ) {
      if( NULL == ps_md_array_iter ) {
         if( NULL == ps_md_arrays ) {
            /* Create the first MD array in the list. */
            ps_md_arrays = calloc( 1, sizeof( MD_ARRAY ) );
            ps_md_arrays->name = calloc( 1, sizeof( char ) );
            ps_md_array_iter = ps_md_arrays;
         } else {
         }
         pc_current_field = ps_md_array_iter->name;
         j = 0;
      }

      if( '<' == pc_md_arrays[i] ) {
         /* Begin collecting a string to split for the dev list. */
         pc_current_field = calloc( 1, sizeof( char ) );
         j = 0;
         i++;
      }

      if( '>' == pc_md_arrays[i] ) {
         /* Convert the gathered string into an array of dev names. */
         ps_md_array_iter->devs = config_split_string_array( pc_current_field );
         free( pc_current_field );

         if( '\0' != pc_md_arrays[i + 1] ) {
            /* Add a new MD array to the list and iterate. */
            ps_md_array_prev = ps_md_array_iter;
            ps_md_array_iter->next = calloc( 1, sizeof( MD_ARRAY ) );
            ps_md_array_iter = ps_md_array_iter->next;
            ps_md_array_iter->name = calloc( 1, sizeof( char ) );
            pc_current_field = ps_md_array_iter->name;

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
      pc_current_field[j] = pc_md_arrays[i];
      pc_current_field[j + 1] = '\0';

      /* Increment. */
      i++;
      j++;
   }

   free( pc_md_arrays );
   
   return ps_md_arrays;
}

void config_free_md_arrays( MD_ARRAY* ps_md_arrays_in ) {
   MD_ARRAY* ps_md_array_iter = ps_md_arrays_in,
      * ps_md_array_prev = NULL;

   while( NULL != ps_md_array_iter ) {
      ps_md_array_prev = ps_md_array_iter;
      ps_md_array_iter = ps_md_array_iter->next;
      free( ps_md_array_prev );
   }
}

