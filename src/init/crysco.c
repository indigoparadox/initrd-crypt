
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
   i_cryptsetup_argc = command_cryptsetup( &ppc_cryptsetup_argv );

   for( i = 0 ; i_lvol_count > i ; i++ ) {
      /* Setup a pipe to send commands to the child. */
      if( pipe( ai_cryptsetup_stdin ) ) {
         #ifdef ERRORS
         perror( "Unable to open communication pipe" );
         #endif /* ERRORS */
         goto ad_cleanup;
      }

      /* Add the current device to cryptsetup args. */
      ppc_cryptsetup_argv[i_cryptsetup_argc] = ap_lvols[i].dev;
      ppc_cryptsetup_argv[i_cryptsetup_argc + 1] = ap_lvols[i].name;

      printf(
         "%s %s %s %s\n",
         ppc_cryptsetup_argv[0],
         ppc_cryptsetup_argv[1],
         ppc_cryptsetup_argv[2],
         ppc_cryptsetup_argv[3]
      );

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

         /* Pipe the password into cryptsetup. */
         if( -1 == write(
            ai_cryptsetup_stdin[1], pc_key_in, strlen( pc_key_in ) )
         ) {
            #ifdef ERRORS
            perror( "Unable to communicate with cryptsetup" );
            #endif /* ERRORS */
            goto ad_cleanup;
         }

         /* while( 0 != waitpid( i_cryptsetup_pid, NULL, WNOHANG ) ) { } */
         wait( NULL );
      }
   }

   /* See if decryption was successful. */
   i_retval = mount_probe_root();

ad_cleanup:

   /* TODO: Kill cryptsetup process if it's active? */

   /* Perform cleanup, destroy the information structure. */
   HOST_FREE_LVOLS( ap_lvols );
   for( i = 0 ; i_cryptsetup_argc > i ; i++ ) {
      free( ppc_cryptsetup_argv[i] );
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
      i_key_attempts = 0,
      i_retval = 0;
   struct termios oldterm,
      newterm;
   
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
         pc_key_buffer[i_key_index] = c_char;
         i_key_index++;
         i_key_buffer_size++;
         pc_key_buffer = realloc( pc_key_buffer, i_key_buffer_size );

         if( '\n' == c_char ) {
            /* Quit on newline, but after it's been added to the buffer. */
            break;
         }
      }

      /* Perform the decryption, passing the resulting retval back. */
      i_retval = attempt_decrypt( pc_key_buffer );

      /* Iteration cleanup. */
      free( pc_key_buffer );
      i_key_buffer_size = 1;
      i_key_index = 0;

      if( !i_retval ) {
         break;
      }

      i_key_attempts++;
   }

   /* Reset terminal to previous (echoing) settings. */
   tcsetattr( fileno( stdin ), TCSANOW, &oldterm );

   return i_retval;
}

