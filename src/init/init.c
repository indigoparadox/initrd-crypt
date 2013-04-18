
#include <stdio.h>
#include <unistd.h>
#include <regex.h>
#include <signal.h>
#include <linux/reboot.h>

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

#ifdef CONSOLE
int action_console( void ) {
   int i_retval = 0;

   /* Just keep spawning a console indefinitely. */
   /* TODO: Enable a graceful exit to boot the system. */
   while( 1 ) {
      system( "/bin/busybox --install && /bin/sh" );
   }

   return i_retval;
}
#endif /* CONSOLE */

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
      /* Switchroot */
      execv( ac_command_switch_root[0], ac_command_switch_root );
   } else {
      /* Reboot */
      reboot( LINUX_REBOOT_CMD_RESTART );
   }

   getchar();

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
   regex_t s_regex;
   int i,
      i_retval = 0;
   char* pc_action_crypt = NULL,
   #ifdef CONSOLE
      * pc_action_console = NULL,
   #endif /* CONSOLE */
      ac_cmdline[CMDLINE_MAX_SIZE] = { '\0' };
   regmatch_t as_match[2];
   FILE* pf_cmdline = NULL;

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

   #if 0
   /* Initialize strings, etc. */
   pc_action_crypt = config_action_crypt();
   #ifdef CONSOLE
   pc_action_console = config_action_console();
   #endif /* CONSOLE */
   if( regcomp( &s_regex, "initdo=\\([a-zA-Z0-9]*\\)", 0 ) ) {
      #ifdef ERRORS
      perror( "Unable to compile cmdline regex" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_REGEX_FAIL;
      goto main_cleanup;
   }

   /* Read the kernel cmdline. */
   /* TODO: Scramble the cmdline path, maybe? */
   pf_cmdline = fopen( "/proc/cmdline", "r" );
   if( NULL == fgets( ac_cmdline, CMDLINE_MAX_SIZE, pf_cmdline ) ) {
      #ifdef ERRORS
      perror( "Unable to read kernel cmdline" );
      #endif /* ERRORS */
      fclose( pf_cmdline );
      goto main_cleanup;
   } else {
      /* Close the cmdline either way. */
      fclose( pf_cmdline );
   }

   /* Act based on the system imperative. */
   if(
      !regexec( &s_regex, ac_cmdline, 2, as_match, 0 ) &&
      !strncmp(
         pc_action_crypt,
         &ac_cmdline[as_match[1].rm_so],
         strlen( pc_action_crypt )
      )
   ) {
   #endif
      i_retval = action_crypt();
   #if 0
   #ifdef CONSOLE
   } else if(
      !regexec( &s_regex, ac_cmdline, 2, as_match, 0 ) &&
      !strncmp(
         pc_action_console,
         &ac_cmdline[as_match[1].rm_so],
         strlen( pc_action_console )
      )
   ) {
      i_retval = action_console();
   #endif /* CONSOLE */
   } else {
      #ifdef ERRORS
      perror( "Invalid or no action specified" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_ACTION_FAIL;
      goto main_cleanup;
   }
   #endif

main_cleanup:
   if( NULL != pc_action_crypt ) {
      free( pc_action_crypt );
   }
   #ifdef CONSOLE
   if( NULL != pc_action_console ) {
      free( pc_action_console );
   }
   #endif /* CONSOLE */

   if( 1 == getpid() ) {
      i_retval = cleanup_system( i_retval );
   }

   return i_retval;
}

