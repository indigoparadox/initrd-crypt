
#include "config_extern.h"

#include "console.h"

int gi_stdout_orig = -1,
   gi_stderr_orig = -1,
   gi_null_fd = -1;

static struct termios* gps_term_echo = NULL;

void console_hide( void ) {
   /* Don't do anything unless we're not currently redirecting. */
   if( -1 == gi_null_fd ) {
      gi_null_fd = open( "/dev/null", O_WRONLY );
   }
   if( -1 == gi_stdout_orig ) {
      fflush( stdout );
      gi_stdout_orig = dup( STDOUT_FILENO );
      dup2( gi_null_fd, STDOUT_FILENO );
   }
   if( -1 == gi_stderr_orig ) {
      fflush( stderr );
      gi_stderr_orig = dup( STDERR_FILENO );
      dup2( gi_null_fd, STDERR_FILENO );
   }
}

void console_show ( void ) {
   /* Don't do anything unless we are currently redirecting. */
   if( -1 != gi_stdout_orig ) {
      dup2( gi_stdout_orig, STDOUT_FILENO );
      gi_stdout_orig = -1;
   }
   if( -1 != gi_stderr_orig ) {
      dup2( gi_stderr_orig, STDERR_FILENO );
      gi_stderr_orig = -1;
   }

   /* Reset the null FD. */
   close( gi_null_fd );
   gi_null_fd = -1;
}

/* The echo system is a little convoluted and probably bug-prone, but suitable for the *
 * uses we expect here. We tried using AND'ing and making it more resiliant, but we    *
 * didn't have much luck and we'd rather work on more interesting things for now.      */
/* TODO: Make console wrapper a little more resiliant. */

void console_echo_off( void ) {
   struct termios s_term_new;

   if( NULL == gps_term_echo ) {
      gps_term_echo = calloc( 1, sizeof( struct termios ) );
      tcgetattr( fileno( stdin ), gps_term_echo );
   }

   s_term_new = *gps_term_echo;

   s_term_new.c_lflag &= ~ECHO;
   tcsetattr( fileno( stdin ), TCSANOW, &s_term_new );
}

void console_echo_on( void ) {
   if( NULL != gps_term_echo ) {  
      tcsetattr( fileno( stdin ), TCSANOW, gps_term_echo );
      free( gps_term_echo );
      gps_term_echo = NULL;
   }
}

#ifdef CONSOLE
int console_shell( void ) {
   int i_retval = 1;

   /* Respawn the console if it crashes, otherwise reboot. */
   while( i_retval ) {
      i_retval = system( "/bin/busybox --install && /bin/sh" );
   }
   i_retval = ERROR_RETVAL_CONSOLE_DONE;

   return i_retval;
}
#endif /* CONSOLE */

char* console_prompt_string( void ) {
   char* pc_key_buffer = NULL,
      c_char;
   int i_key_buffer_size = 1,
      i_key_index = 0;

   pc_key_buffer = calloc( i_key_buffer_size, sizeof( char ) );
   while( (c_char = getchar()) ) {
      if( '\n' == c_char && 1 != i_key_buffer_size ) {
         break;
      }

      /* TODO: Handle backspace/delete properly. */
      pc_key_buffer[i_key_index] = c_char;
      i_key_index++;
      i_key_buffer_size++;
      pc_key_buffer = realloc( pc_key_buffer, i_key_buffer_size );
      pc_key_buffer[i_key_index] = '\0'; /* Ensure NULL termination. */
   }

   return pc_key_buffer;
}

