
#include "config_extern.h"

#include "crysco.h"

/* Purpose: Prompt for keys for attempts up to HOST_MAX_ATTEMPTS and call the *
 *          decryption attempt routine for each key provided.                 */
/* Return: 0 on success, 1 on failure.                                        */
int prompt_decrypt( void ) {
   char* pc_key_buffer;
   int i_key_attempts = 0,
      i_retval = 0;
   
   while( CONFIG_MAX_ATTEMPTS > i_key_attempts ) {

      /* Disable password echo. */
      console_echo_off();

      /* Get a password from stdin. */
      printf( "Insufficient data.\n" );
      pc_key_buffer = console_prompt_string();

      /* Echo in case we drop to console or something. */
      console_echo_on();

      #if DEBUG
      printf( "KEY: \"%s\"\n", pc_key_buffer );
      #endif /* DEBUG */

      /* Perform the decryption, passing the resulting retval back. */
      i_retval = mount_decrypt( pc_key_buffer );
      /* See if decryption was successful. */
      if( !i_retval ) {
         mount_probe_lvm();
         i_retval |= mount_probe_root();
      }

      #if 0
      if( !strcmp( pc_key_buffer, "foo" ) ) {
         i_retval = 0;
      } else {
         i_retval = 1;
      }
      #endif

      /* Iteration cleanup. */
      free( pc_key_buffer );

      /* Break the loop to boot or reboot on certain errors. */
      if( 
         !i_retval ||
         (ERROR_RETVAL_ROOT_FAIL & i_retval) ||
         (ERROR_RETVAL_MAPPER_FAIL & i_retval)
      ) {
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

