
#include "crysco.h"

/* Purpose: Attempt to decrypt encrypted volumes for this host.               */
/* Return: 0 on success, 1 on failure.                                        */
int attempt_decrypt( char* pc_key_in ) {
   int i,
      i_lvol_count,
      i_retval = 0,
      i_cryptsetup_pid,
      ai_cryptsetup_stdin[2],
      i_cryptsetup_argc;
   char** ppc_cryptsetup_argv = NULL;
   LVOL* ap_lvols;

   i_lvol_count = host_lvols( &ap_lvols );
   i_cryptsetup_argc =
      command_crypstsetup( &ppc_cryptsetup_argv );

   for( i = 0 ; i_lvol_count > i ; i++ ) {
      /* FIXME: Actually attempt decryption. */
      /* printf( "%s", ap_lvols[i_lvol_iter].name ); */

      /* # Try to mount the disks given the entered password.
      for BLK_MNT_ITER in `/bin/grep "^M_LK" "/etc/cryscry.cfg"`; do
         DEV_ITER="`echo "$BLK_MNT_ITER" | /bin/awk -F'=' '{print $2}'`"
         MAP_ITER="`echo "$BLK_MNT_ITER" | /bin/awk -F'=' '{print $3}'`"
         echo "$P" | /sbin/cryptsetup luksOpen "$DEV_ITER" "$MAP_ITER"

         #if [ 0 = $? ] && [ 1 = $CFG_VERBOSE ]; then
         #   echo "Success."
         #elif [ 0 = $? ] && [ 1 = $CFG_VERBOSE ]; then
         #   echo "Failure."

         if [ 0 -ne $? ]; then
            # If even one device can't be unlocked then try again.
            PP_ATTEMPTS=$(( $PP_ATTEMPTS+1 ))
            PP_ATTEMPT_GOOD=0
            break
         fi
      done */

      /* Setup a pipe to send commands to the child. */
      if( pipe( ai_cryptsetup_stdin ) ) {
         #ifdef ERRORS
         perror( "Unable to open communication pipe" );
         #endif /* ERRORS */
         goto ad_cleanup;
      }

      /* Fork and start cryptsetup, passing it the password entered. */
      i_cryptsetup_pid = fork();
      if( 0 == i_cryptsetup_pid ) {
         /* This is the child process. */

         /* Close stdout/stderr and attach parent's pipe stdin. */
         close( 1 );
         close( 2 );
         close( 0 );
         dup2( ai_cryptsetup_stdin[0], 0 );

         execv( ppc_cryptsetup_argv[0], ppc_cryptsetup_argv );

      } else if( 0 < i_cryptsetup_pid ) {
         /* This is the parent process. */

         close( ai_cryptsetup_stdin[0] );

         /* Wait until we get SIGTERM or the child dies. */
         wait( NULL );
      }
   }

   /* See if decryption was successful. */
   if( mount_probe_root() ) {
      i_retval = 1;
      goto ad_cleanup;
   }

ad_cleanup:

   /* Perform cleanup, destroy the information structure. */
   HOST_FREE_LVOLS( ap_lvols );

   i = 0;
   while( NULL != ppc_cryptsetup_argv[i] ) {
      free( ppc_cryptsetup_argv[i] );
      i++;
   }
   free( ppc_cryptsetup_argv );

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
      i_key_attempts = 0;
   struct termios oldterm,
      newterm;
   BOOL b_decrypt_success = FALSE;
   
   /* Disable local echo. */
   tcgetattr( fileno( stdin ), &oldterm );
   newterm = oldterm;
   newterm.c_lflag &= ~ECHO;
   tcsetattr( fileno( stdin ), TCSANOW, &newterm );

   while( host_max_attempts() > i_key_attempts ) {

      /* Get a password from stdin. */
      pc_key_buffer = calloc( i_key_buffer_size, sizeof( char ) );
      printf( "Insufficient data.\n" );
      while( (c_char = getchar()) ) {
         if( '\n' == c_char ) {
            /* Quit on newline. */
            break;
         }

         pc_key_buffer[i_key_index] = c_char;
         i_key_index++;
         i_key_buffer_size++;
         pc_key_buffer = realloc( pc_key_buffer, i_key_buffer_size );
      }

      attempt_decrypt( pc_key_buffer );

      /* Iteration cleanup. */
      free( pc_key_buffer );

      i_key_attempts++;
   }

   return !b_decrypt_success;
}

