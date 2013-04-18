
#include "config.h"

/*
const char gac_skey[] = { CONFIG_SKEY };

const char gac_re_md_arrays[] = { CONFIG_REGEX_MD_ARRAYS };
const char gac_re_string_array[] = { CONFIG_REGEX_STRING_ARRAY };

const char gac_md_arrays[] = { CONFIG_MD_ARRAYS };
const char gac_luks_vols[] = { CONFIG_LUKS_VOLS };
const char gac_net_if[] = { CONFIG_NET_IF };
const char gac_net_ip[] = { CONFIG_NET_IP };
const char gac_sys_fs_mount[] = { CONFIG_SYS_FS_MOUNT };
const char gac_sys_mtype_mount[] = { CONFIG_SYS_MTYPE_MOUNT };
const char gac_sys_fs_umount[] = { CONFIG_SYS_FS_UMOUNT };
const char gac_sys_mpoint_root[] = { CONFIG_SYS_MPOINT_ROOT };
const char gac_sys_path_mapper[] = { CONFIG_SYS_PATH_MAPPER };
const char gac_command_mdadm[] = { CONFIG_COMMAND_MDADM };
*/

/* = Configuration Helpers = */

/* Notes:   This function is for internal use within this module only.        *
 *          Properties should be accessed through a specialized configuration *
 *          loader function defined below.                                    */
static char config_descramble_char( char x_in, char y_in ) {
   return ~((x_in & y_in) | (~x_in & ~y_in));
}

/* Purpose: Take a hard-coded scrambled string and return a dynamically-      *
 *          allocated descrambled string pointer.                             */
char* config_descramble_string( const char* pc_string_in ) {
   char c,
      *pc_out;
   int i = 0;

   pc_out = calloc( strlen( pc_string_in ) + 1, sizeof( char ) );

   while( '\0' != (c = pc_string_in[i]) ) {

      pc_out[i] = config_descramble_char( pc_string_in[i], gac_skey[i] );

      /* Iterate. */
      i++;
   }

   return pc_out;
}

/* Notes:   The string passed as pc_string_in should be free()ed afterwards.  */
char** config_split_string_array( const char* pc_string_in, char* pc_re_in ) {
   regex_t s_regex;
   regmatch_t as_match[CONFIG_STRING_ARRAY_MAX_LEN];
   char** ppc_out = NULL,
      c_free_re_on_exit = 0;
   int i, j,
      i_strlen;

   if( NULL == pc_re_in ) {
      /* No regex was specified, so use the standard string-splitting regex. */
      pc_re_in = config_descramble_string( gac_re_string_array );
      c_free_re_on_exit = 1;
   }

   /* Initialize regex engine. */
   if( regcomp( &s_regex, pc_re_in, 0 ) ) {
      #ifdef ERRORS
      perror( "Unable to compile string-splitting regex" );
      #endif /* ERRORS */
      goto cssa_cleanup;
   }

   ppc_out = calloc( 1, sizeof( char* ) );
   if( !regexec(
      &s_regex, pc_string_in, CONFIG_STRING_ARRAY_MAX_LEN, &as_match, 0
   ) ) {
      printf( "rt: %s\n", pc_string_in );
      for( i = 0 ; CONFIG_STRING_ARRAY_MAX_LEN > i ; i++ ) {
         i_strlen = as_match[i].rm_eo - as_match[i].rm_so;
         if( 1 > i_strlen ) {
            /* Invalid match. */
            break;
         }

         printf( "rd: %d %d\n", as_match[i].rm_so, as_match[i].rm_eo );

         /* Add another string to the array. */
         ppc_out = realloc( ppc_out, (i + 1) * sizeof( char* ) );
         ppc_out[i] = calloc( i_strlen + 1, sizeof( char ) );
         for( j = 0 ; j < i_strlen ; j++ ) {
            ppc_out[i][j] = pc_string_in[as_match[i].rm_so + i];
         }
      }

      /* Add a NULL tag at the end of the array. */
      ppc_out = realloc( ppc_out, (i + 1) * sizeof( char* ) );
      ppc_out[i] = NULL;
   } else {
      printf( "BAD REGEX\n" );
   }

cssa_cleanup:

   if( c_free_re_on_exit ) {
      /* Only leave behind what we didn't bring with us. */
      free( pc_re_in );
   }

   return ppc_out;
}

/* = Configuration Loaders = */

MD_ARRAY* config_load_md_arrays( void ) {
   MD_ARRAY* ps_md_arrays_out = NULL,
      * ps_md_array_iter = NULL;
   /* TODO: How should we decide the max potential matches? */
   char* pc_re_md_arrays_outer = NULL,
      * pc_re_md_arrays_inner = NULL,
      * pc_md_arrays = NULL;
   int i_retval,
      i_strlen,
      i;


   /* Parse out the arrays and create structs for them. */

   #if 0
   //md_arrays = calloc( 1, sizeof( MD_ARRAY ) );
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

   free( pc_re_md_arrays_outer );
   free( pc_re_md_arrays_inner );
   
   return ps_md_arrays_out;
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

