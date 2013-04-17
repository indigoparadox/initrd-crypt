
#include "config.h"

char gac_skey[] = { CONFIG_SKEY };

char gac_re_md_arrays[] = { CONFIG_REGEX_MD_ARRAYS };
char gac_re_string_array[] = { CONFIG_REGEX_STRING_ARRAY };

char gac_md_arrays[] = { CONFIG_MD_ARRAYS };
char gac_luks_vols[] = { CONFIG_LUKS_VOLS };
char gac_net_if[] = { CONFIG_NET_IF };
char gac_net_ip[] = { CONFIG_NET_IP };

/* = Configuration Helpers = */

static char descramble_char( char x_in, char y_in ) {
   return ~((x_in & y_in) | (~x_in & ~y_in));
}

/* Purpose: Take a hard-coded scrambled string and return a dynamically-      *
 *          allocated descrambled string pointer.                             *
 * Notes:   This function is for internal use within this module only.        *
 *          Properties should be accessed through a specialized configuration *
 *          loader function defined below.                                    */
static char* config_load_string( char* pc_string_in ) {
   char c,
      *pc_out;
   int i = 0;

   pc_out = calloc( strlen( pc_string_in ), sizeof( char ) );

   while( '\0' != (c = pc_string_in[i]) ) {

      pc_out[i] = read_char_static( pc_string_in[i], gac_skey[i] );

      /* Iterate. */
      i++;
   }

   return pc_out;
}

/* = Configuration Loaders = */

MD_ARRAY* config_load_md_arrays( void ) {
   MD_ARRAY* ps_md_arrays_out = NULL,
      * ps_md_array_iter = NULL;

   md_arrays = calloc( 1, sizeof( MD_ARRAY ) );

   /* Parse out the arrays 

   #if 0
   for( i = 0 ; host_md_count() > i ; i++ ) {
      (*md_arrays)[i].name = descramble_create_string( \
         aac_dev_md_names[i], gi_skey \
      ); \
      /* TODO: Allow for md devices with varying count of devs. */ \
      (*md_arrays)[i].devs_count = host_md_devs_per(); \
      (*md_arrays)[i].devs = calloc( host_md_devs_per(), sizeof( char* ) ); \
      for( j = 0 ; host_md_devs_per() > j ; j++ ) { \
         (*md_arrays)[i].devs[j] = \
            descramble_create_string( aac_dev_md_devs[i][j], gi_skey ); \
      } \
   }
   #endif

clma_cleanup:
   
   return ps_md_arrays_out;
}

void config_free_md_arrays( MD_ARRAY* ps_md_arrays_in ) {

}

/* static void config_destroy_string( char** ppc_string_in ) {

} */

#if 0
void descramble_copy_string( char* dest_in, char* string_in, char* key_in ) {
}

/* Purpose: Create a dynamic descrambled copy of the given scrambled string.  */
/* Return: A pointer to the new string. Must be freed manually.               */
char* descramble_create_string( char* string_in, char* key_in ) {
   char* pc_out;

   pc_out = calloc( strlen( string_in ), sizeof( char ) );
   descramble_copy_string( pc_out, string_in, key_in );

   return pc_out;
}
#endif

