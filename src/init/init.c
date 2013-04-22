
#include "config_extern.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"

#include "crysco.h"
#include "mount.h"
#include "network.h"

/* Purpose: Wait until the main devices are decrypted and start the system.   */
int action_crypt( void ) {

   #ifdef NET
   /* TODO: Try to start network listener. */
   #endif /* NET */

   /* Get the user password. */
   return prompt_decrypt();
}

/* Purpose: Tidy up the system and prepare/enact the "real" boot process.     *
 *          This should only be called from init/pid 1.                       */
int cleanup_system( int i_retval_in ) {
   char* pc_command_switch_root_string,
      ** ppc_command_switch_root;

   pc_command_switch_root_string = config_descramble_string(
      gac_command_switch_root,
      gai_command_switch_root
   );
   ppc_command_switch_root = config_split_string_array(
      pc_command_switch_root_string
   );

   #if defined DEBUG && defined CONSOLE
   if( i_retval_in ) {
      console_shell();
   }
   #endif /* DEBUG, CONSOLE */

   #ifdef NET
   /* TODO: Try to stop network. */
   /* killall dropbear 2>/dev/null
   for DEV_ITER in `/bin/cat /proc/net/dev | /bin/awk '{print $1}' | \
      /bin/grep ":$" | /bin/sed s/.$//`
   do
      ifconfig $DEV_ITER down 2>/dev/null
   done */
   #endif /* NET */

   #ifdef SERIAL
   /* TODO: Try to stop serial port. */
   stop_serial();
   #endif /* SERIAL */

   /* Prepare the system to load the "real" init (or reboot). */
   if( !i_retval_in ) {
      /* TODO: Start OR'ing return values instead of reassigning them. */
      i_retval_in |= mount_probe_usr();
   }
   umount_sys();

   /* Execute switchroot on success, reboot on failure. */
   if( !i_retval_in ) {
      #ifdef DEBUG
      printf( "Boot ready.\n" );
      getchar();
      #endif /* DEBUG */

      /* Switchroot */
      execv( ppc_command_switch_root[0], ppc_command_switch_root );
   } else {
      #ifdef DEBUG
      printf( "Boot failed.\n" );
      getchar();
      #endif /* DEBUG */

      /* Reboot */
      reboot( LINUX_REBOOT_CMD_RESTART );
   }

   /* Meaningless cleanup routines. */
   free( pc_command_switch_root_string );
   config_free_string_holders( ppc_command_switch_root );

   return i_retval_in;
}

void signal_handler( int i_signum_in ) {
   if( SIGTERM == i_signum_in && 1 == getpid()  ) {
      /* A successful decryption occurred on a remote terminal, so shut       *
       * everything down.                                                     */
      cleanup_system( 0 );
   } else if( SIGTERM != i_signum_in ) {
      switch( i_signum_in ) {
         case SIGQUIT:
         case SIGINT:
            /* Do nothing. */
            break;
      }
   }
}

int main( int argc, char* argv[] ) {
   int i,
      i_retval = 0,
      i_retval_local = 0;

   /* Protect ourselves against simple potential bypasses. */
   signal( SIGTERM, signal_handler );
   signal( SIGINT, signal_handler );
   signal( SIGQUIT, signal_handler );

   if( 1 == getpid() ) {
      /* We're being called as init, so set the system up. */
      i_retval = mount_sys();
      if( i_retval ) {
         goto main_cleanup;
      }
      i_retval = mount_mds();
      if( i_retval ) {
         goto main_cleanup;
      }
      #ifdef NET
      i_retval = setup_network();
      if( i_retval ) {
         goto main_cleanup;
      }
      #endif /* NET */

      #ifdef SERIAL
      i_retval = setup_serial();
      if( i_retval ) {
         goto main_cleanup;
      }
      #endif /* SERIAL */

      /* TODO: Load any directed kernel modules. */

      /* TODO: Start the splash screen (deprecated). */
   } else {
      /* We're being called as a sub-prompt, so just prompt to decrypt and    *
       * exit on success or failure.                                          */

      i_retval = prompt_decrypt();
      if( !i_retval ) {
         #ifdef DEBUG
         printf( "Killing init. Press any key.\n" );
         getchar();
         #endif /* DEBUG */
         i_retval_local = kill( 1, SIGTERM );
         if( i_retval_local ) {
            /* This isn't as important as the errorlevel we're exiting with. */
            #ifdef ERRORS
            perror( "Could not kill init" );
            #endif /* ERRORS */
         }
      }
      goto main_cleanup;
   }

   /* Start the challenge! */
   i_retval = action_crypt();

main_cleanup:

   if( 1 == getpid() ) {
      i_retval = cleanup_system( i_retval );
   }

   return i_retval;
}

