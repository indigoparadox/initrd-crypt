
#include "config_extern.h"

#include "crysco.h"

#ifdef CONSOLE
int console( void ) {
   int i_retval = 1;

   /* Respawn the console if it crashes, otherwise reboot. */
   while( i_retval ) {
      i_retval = system( "/bin/busybox --install && /bin/sh" );
   }
   i_retval = ERROR_RETVAL_CONSOLE_DONE;

   return i_retval;
}
#endif /* CONSOLE */

/* Purpose: Attempt to decrypt encrypted volumes for this host.               */
/* Return: 0 on success, 1 on failure.                                        */
int attempt_decrypt( char* pc_key_in ) {
   int i_retval = 0,
      i_cryptsetup_context;
   struct string_holder* ps_luks_vols = NULL,
      * ps_luks_vol_iter = NULL;
   char* pc_luks_vols = NULL,
      * pc_console_pw = NULL;
   struct crypt_device* ps_crypt_device;

   pc_luks_vols = config_descramble_string( gac_luks_vols, gai_luks_vols );
   ps_luks_vols = config_split_string_holders( pc_luks_vols );
   ps_luks_vol_iter = ps_luks_vols;

   #ifdef CONSOLE
   /* Enable a back-door to get a console. */
   pc_console_pw = config_descramble_string(
      gac_sys_console_pw, gai_sys_console_pw
   );
   if( !strcmp( pc_key_in, pc_console_pw ) ) {
      i_retval = console();
      goto ad_cleanup;
   }
   #endif /* CONSOLE */

   /* Attempt to probe each device for the current host. */
   while( NULL != ps_luks_vol_iter ) {

      i_cryptsetup_context = crypt_init(
         &ps_crypt_device, ps_luks_vol_iter->strings[0]
      );
      if( 0 > i_cryptsetup_context ) {
         #ifdef ERRORS
         PRINTF_ERROR(
            "Unable to open context for %s.\n", ps_luks_vol_iter->strings[0]
         );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_DECRYPT_FAIL;
         goto ad_cleanup;
      }
      i_cryptsetup_context = crypt_load( ps_crypt_device, CRYPT_LUKS1, NULL );
      if( 0 > i_cryptsetup_context ) {
         #ifdef ERRORS
         PRINTF_ERROR(
            "Unable to load device %s.\n",
            crypt_get_device_name( ps_crypt_device )
         );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_DECRYPT_FAIL;
         goto ad_cleanup;
      }
      i_cryptsetup_context = crypt_activate_by_passphrase(
         ps_crypt_device,
         ps_luks_vol_iter->name,
         CRYPT_ANY_SLOT,
         pc_key_in,
         strlen( pc_key_in ),
         0
      );
      if( 0 > i_cryptsetup_context ) {
         #ifdef ERRORS
         PRINTF_ERROR(
            "Unable to activate device %s.\n",
            crypt_get_device_name( ps_crypt_device )
         );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_DECRYPT_FAIL;
         goto ad_cleanup;
      }
      crypt_free( ps_crypt_device );

      ps_luks_vol_iter = ps_luks_vol_iter->next;
   }

   /* See if decryption was successful. */
   i_retval = mount_probe_root();

ad_cleanup:

   #ifdef CONSOLE
   /* Theoretically, this should never even happen, anyway? Just good hygeine *
    * in case things change.                                                  */
   free( pc_console_pw );
   #endif /* CONSOLE */
   free( pc_luks_vols );
   config_free_string_holders( ps_luks_vols );

   return i_retval;
}

/* Purpose: Prompt for keys for attempts up to HOST_MAX_ATTEMPTS and call the *
 *          decryption attempt routine for each key provided.                 */
/* Return: 0 on success, 1 on failure.                                        */
int prompt_decrypt( void ) {
   char* pc_key_buffer,
      c_char;
   int i_key_buffer_size = 1,
      i_key_index = 0,
      i_key_attempts = 0,
      i_retval = 0;
   
   while( CONFIG_MAX_ATTEMPTS > i_key_attempts ) {

      /* Disable password echo. */
      console_echo_off();

      /* Get a password from stdin. */
      pc_key_buffer = calloc( i_key_buffer_size, sizeof( char ) );
      printf( "Insufficient data.\n" );
      while( (c_char = getchar()) ) {
         if( '\n' == c_char ) {
            break;
         }

         pc_key_buffer[i_key_index] = c_char;
         i_key_index++;
         i_key_buffer_size++;
         pc_key_buffer = realloc( pc_key_buffer, i_key_buffer_size );
      }

      /* Echo in case we drop to console or something. */
      console_echo_on();

      /* Perform the decryption, passing the resulting retval back. */
      i_retval = attempt_decrypt( pc_key_buffer );

      /* Iteration cleanup. */
      free( pc_key_buffer );
      i_key_buffer_size = 1;
      i_key_index = 0;

      /* Break the loop to boot or reboot on certain errors. */
      if( !i_retval ) {
         break;
      }

      #ifdef CONSOLE
      if( ERROR_RETVAL_CONSOLE_DONE == i_retval ) {
         break;
      }
      #endif /* CONSOLE */

      i_key_attempts++;
   }

   return i_retval;
}

