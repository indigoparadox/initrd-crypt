
#include "crysco.h"

/* Purpose: Attempt to decrypt encrypted volumes for this host.               */
/* Return: 0 on success, 1 on failure.                                        */
int attempt_decrypt( char* pc_key_in ) {
   int i_lvol_iter,
      i_lvol_count;
   LVOL* ap_lvols;

   i_lvol_count = get_lvols( &ap_lvols );

   for( i_lvol_iter = 0 ; i_lvol_count > i_lvol_iter ; i_lvol_iter++ ) {
      /* FIXME: Actually attempt decryption. */
      /* printf( "%s", ap_lvols[i_lvol_iter].name ); */
   }

   /* Perform cleanup, destroy the information structure. */
   for( i_lvol_iter = 0 ; i_lvol_count > i_lvol_iter ; i_lvol_iter++ ) {
      free( ap_lvols[i_lvol_iter].name );
      free( ap_lvols[i_lvol_iter].dev );
      free( ap_lvols );
   }

   /* See if decryption was successful. */
   if( mount_probe_root() ) {
      return 1;
   }

   return 0;
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

   while( HOST_MAX_ATTEMPTS > i_key_attempts ) {

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

      printf( "%s\n", pc_key_buffer );

      attempt_decrypt( pc_key_buffer );

      /* Iteration cleanup. */
      free( pc_key_buffer );

      i_key_attempts++;
   }

   return !b_decrypt_success;
}

