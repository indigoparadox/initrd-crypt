
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"

#include "crysco.h"
#include "mount.h"

#define CMDLINE_MAX_SIZE 255

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
   char* ac_command_switch_root[] = {
      "switch_root",
      "/mnt/root",
      "/sbin/init"
   };

   #ifdef NET
   /* TODO: Try to stop network. */
   /* killall dropbear 2>/dev/null
   for DEV_ITER in `/bin/cat /proc/net/dev | /bin/awk '{print $1}' | \
      /bin/grep ":$" | /bin/sed s/.$//`
   do
      ifconfig $DEV_ITER down 2>/dev/null
   done */
   #endif /* NET */

   /* Prepare the system to load the "real" init. */
   if( !i_retval_in ) {
      i_retval_in = mount_probe_usr();
   }
   if( !i_retval_in ) {
      i_retval_in = umount_sys();
   }

   /* Execute switchroot on success, reboot on failure. */
   if( !i_retval_in ) {
      #ifdef DEBUG
      printf( "Boot ready.\n" );
      getchar();
      #endif /* DEBUG */

      /* Switchroot */
      execv( ac_command_switch_root[0], ac_command_switch_root );
   } else {
      #ifdef DEBUG
      printf( "Boot failed.\n" );
      getchar();
      #endif /* DEBUG */

      /* Reboot */
      reboot( LINUX_REBOOT_CMD_RESTART );
   }

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
      i_retval = 0;

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

      /* TODO: Load any directed kernel modules. */

      /* TODO: Start the splash screen (deprecated). */
   }

   /* See if we're being called as a prompt only. */
   for( i = 1 ; i < argc ; i++ ) {
      if( !strncmp( "-p", argv[i], 2 ) ) {
         /* Just prompt to decrypt and exit (signaling main process to clean  *
          * up if decrypt is successful!)                                     */
         i_retval = prompt_decrypt();
         if( !i_retval ) {
            kill( 1, SIGUSR1 );
         }
         goto main_cleanup;
      }
   }

   /* Start the challenge! */
   i_retval = action_crypt();

main_cleanup:

   if( 1 == getpid() ) {
      i_retval = cleanup_system( i_retval );
   }

   return i_retval;
}

