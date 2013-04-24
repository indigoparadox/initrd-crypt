
#include "config_extern.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"

#include "crysco.h"
#include "mount.h"
#include "network.h"
#include "error.h"

/* Purpose: Tidy up the system and prepare/enact the "real" boot process.     *
 *          This should only be called from init/pid 1.                       */
void cleanup_system( int i_retval_in ) {

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
   ERROR_PRINTF(
      stop_serial(),
      i_retval_in,
      ERROR_RETVAL_SERIAL_FAIL,
      boot_failed,
      "Unable to stop serial subsystem.\n"
   );
   #endif /* SERIAL */

   /* Prepare the system to load the "real" init (or reboot). */
   ERROR_PRINTF(
      mount_probe_usr(),
      i_retval_in,
      ERROR_RETVAL_SYSFS_FAIL,
      boot_failed,
      "Unable to detect or mount usr directory.\n"
   );

   ERROR_PRINTF(
      umount_sys(),
      i_retval_in,
      ERROR_RETVAL_SYSFS_FAIL,
      boot_failed,
      "Unable to unmount system directories.\n"
   );

   /* Execute switchroot on success, reboot on failure. */
   ERROR_PRINTF(
      mount_switch_root( "/mnt/root" ),
      i_retval_in,
      ERROR_RETVAL_ROOT_FAIL,
      boot_failed,
      "Unable to chroot.\n"
   );

boot_failed:
   
   if( i_retval_in ) {
      #ifdef DEBUG
      printf( "Boot failed.\n" );
      getchar();
      #endif /* DEBUG */

      /* Reboot */
      reboot( LINUX_REBOOT_CMD_RESTART );
   }
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
   int i_retval = 0,
      i_retval_local = 0;

   /* Protect ourselves against simple potential bypasses. */
   signal( SIGTERM, signal_handler );
   signal( SIGINT, signal_handler );
   signal( SIGQUIT, signal_handler );

   if( 1 == getpid() ) {
      /* We're being called as init, so set the system up. */
      ERROR_PRINTF(
         mount_sys(),
         i_retval,
         ERROR_RETVAL_SYSFS_FAIL,
         main_cleanup,
         "There was a problem mounting dynamic system filesystems.\n"
      );

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
   i_retval = prompt_decrypt();

main_cleanup:

   if( 1 == getpid() ) {
      cleanup_system( i_retval );
   }

   return i_retval;
}

