
#include "config_extern.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"

#include "crysco.h"
#include "mount.h"
#include "network.h"
#include "error.h"

#define READ_CMDLINE_BUFFER 4096

char* gpc_serial_listen = NULL;
extern int gi_serial_port;

/* Purpose: Tidy up the system and prepare/enact the "real" boot process.     *
 *          This should only be called from init/pid 1.                       */
void cleanup_system( int i_retval_in ) {
   char* pc_mpoint_root;

   pc_mpoint_root = config_descramble_string(
      gac_sys_mpoint_root,
      gai_sys_mpoint_root
   );

   /* TODO: Come up with a more uniform way to see if we skipped most setup. */
   if( i_retval_in && ERROR_RETVAL_SHUTDOWN != i_retval_in ) {
      /* Drop to console or whatever without trying to clean up if we already *
       * have an error.                                                       */
      goto boot_failed;
   }

   /* Prepare the system to load the "real" init (or reboot). */

   #ifdef NET
   #ifdef TOR
   ERROR_PRINTF(
      network_stop_tor(),
      i_retval_in,
      ERROR_RETVAL_TOR_FAIL,
      boot_failed,
      "Unable to stop tor daemon.\n"
   );
   #endif /* TOR */

   ERROR_PRINTF(
      network_stop_ssh(),
      i_retval_in,
      ERROR_RETVAL_SSH_FAIL,
      boot_failed,
      "Unable to stop SSH daemon.\n"
   );

   ERROR_PRINTF(
      stop_network(),
      i_retval_in,
      ERROR_RETVAL_NET_FAIL,
      boot_failed,
      "Unable to stop network.\n"
   );
   #endif /* NET */

   ERROR_PRINTF(
      umount_sys(),
      i_retval_in,
      ERROR_RETVAL_SYSFS_FAIL,
      boot_failed,
      "Unable to unmount system directories.\n"
   );

   /* Perform alternate commands. */
   if( ERROR_RETVAL_SHUTDOWN & i_retval_in ) {
      reboot( LINUX_REBOOT_CMD_POWER_OFF );
      exit( 0 );
   }

   /* Execute switchroot on success, reboot on failure. */
   ERROR_PRINTF(
      mount_switch_root( pc_mpoint_root ),
      i_retval_in,
      ERROR_RETVAL_ROOT_FAIL,
      boot_failed,
      "Unable to chroot.\n"
   );

boot_failed:

   free( pc_mpoint_root );

   #if defined DEBUG && defined CONSOLE
   if( i_retval_in ) {
      console_shell();
   }
   #endif /* DEBUG, CONSOLE */
   
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
      i_retval_local = 0,
      i_cmdline = 0;
   struct stat s_stat;
   dev_t i_root_dev;
   char ac_cmdline[READ_CMDLINE_BUFFER] = { 0 };
   regmatch_t pmatch[2];
   regex_t s_regex;

   /* Protect ourselves against simple potential bypasses. */
   signal( SIGTERM, signal_handler );
   signal( SIGINT, signal_handler );
   signal( SIGQUIT, signal_handler );

   if( 1 == getpid() ) {
      stat( "/", &s_stat );
      i_root_dev = s_stat.st_dev;
      mount_chown_root( "/", i_root_dev );

      /* We're being called as init, so set the system up. */
      ERROR_PRINTF(
         mount_sys(),
         i_retval,
         ERROR_RETVAL_SYSFS_FAIL,
         main_cleanup,
         "There was a problem mounting dynamic system filesystems.\n"
      );

      PRINTF_DEBUG( "Setting up md devices...\n" );
      i_retval = mount_mds();
      if( i_retval ) {
         goto main_cleanup;
      }
      #ifdef NET
      PRINTF_DEBUG( "Setting up network...\n" );
      i_retval = setup_network();
      if( i_retval ) {
         goto main_cleanup;
      }

      i_retval |= network_start_ssh();
      if( i_retval ) {
         goto main_cleanup;
      }

      #ifdef TOR
      ERROR_PRINTF(
         network_start_tor(),
         i_retval,
         ERROR_RETVAL_TOR_FAIL,
         main_cleanup,
         "Unable to start tor daemon.\n"
      );
      #endif /* TOR */
      #endif /* NET */

      /* TODO: Load any directed kernel modules. */

      /* TODO: Start the splash screen (deprecated). */
   }

   /* TODO: Use ERROR_PRINTF() here. */
   if( regcomp( &s_regex, "ifdy=\\([a-zA-Z0-9]*\\)", 0 ) ) {
      #ifdef ERRORS
      perror( "Unable to compile cmdline regex" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_REGEX_FAIL;
      goto main_cleanup;
   }

   /* Read the kernel cmdline. */
   i_cmdline = open( "/proc/cmdline", O_RDONLY );
   if(
      0 < i_cmdline &&
      0 > read( i_cmdline, ac_cmdline, READ_CMDLINE_BUFFER )
   ) {
      #ifdef ERRORS
      perror( "Unable to read kernel cmdline" );
      #endif /* ERRORS */
   } else {
      PRINTF_DEBUG( "Kernel command line: %s\n", ac_cmdline );
   }
   close( i_cmdline );

   /* Act based on the system imperative. */
   if(
      !regexec( &s_regex, ac_cmdline, 2, pmatch, 0 ) &&
      !strncmp(
         "shutdown",
         &ac_cmdline[pmatch[1].rm_so],
         strlen( "shutdown" )
      )
   ) {
      /* Give some time for daemons to start and create pid files. */
      sleep( 1 );
      i_retval |= ERROR_RETVAL_SHUTDOWN;
   } else {
      /* Start the challenge! */
      i_retval = prompt_decrypt();
   }

   if( 1 != getpid() && !i_retval ) {
      /* We're being called as a sub-prompt, so just prompt to decrypt and    *
       * exit on success or failure.                                          */
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
         goto main_cleanup;
      }
   }

main_cleanup:

   regfree( &s_regex );

   if( 1 == getpid() ) {
      cleanup_system( i_retval );
   }

   return i_retval;
}

