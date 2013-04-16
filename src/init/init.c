
#include <stdio.h>
#include <unistd.h>
#include <regex.h>

#include "host.h"

#ifndef HOSTNAME_H
#error "No hostname specified!"
#endif /* HOSTNAME */

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

   /* TODO: Implement console. */
   /* echo "Do your best!"
   /bin/busybox --install && /bin/sh */

   return 0;
}
#endif /* CONSOLE */

int main( int argc, char* argv[] ) {
   regex_t s_regex;
   int i,
      i_retval = 0;
   char* pc_action_crypt = NULL,
   #ifdef CONSOLE
      * pc_action_console = NULL,
   #endif /* CONSOLE */
      ac_cmdline[CMDLINE_MAX_SIZE] = { '\0' };
   regmatch_t pmatch[2];
   FILE* pf_cmdline = NULL;

   if( 1 == getpid() ) {
      /* We're being called as init, so set the system up. */
      i_retval = mount_sys( FALSE );
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
         if( prompt_decrypt() ) {
         }
         goto main_cleanup;
      }
   }

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
      !regexec( &s_regex, ac_cmdline, 2, pmatch, 0 ) &&
      !strncmp(
         pc_action_crypt,
         &ac_cmdline[pmatch[1].rm_so],
         strlen( pc_action_crypt )
      )
   ) {
      i_retval = action_crypt();
   #ifdef CONSOLE
   } else if(
      !regexec( &s_regex, ac_cmdline, 2, pmatch, 0 ) &&
      !strncmp(
         pc_action_console,
         &ac_cmdline[pmatch[1].rm_so],
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

main_cleanup:
   if( 1 == getpid() ) {
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
      if( !i_retval ) {
         i_retval = mount_probe_usr();
      }
      if( !i_retval ) {
         i_retval = mount_sys( FALSE );
      }

      /* FIXME: Execute switchroot. */
      if( !i_retval ) {
      }
   }

   if( NULL != pc_action_crypt ) {
      free( pc_action_crypt );
   }
   #ifdef CONSOLE
   if( NULL != pc_action_console ) {
      free( pc_action_console );
   }
   #endif /* CONSOLE */

   return i_retval;
}

