
#include "config_extern.h"

#include "console.h"

int gi_stdout_orig = -1,
   gi_stderr_orig = -1,
   gi_null_fd = -1;

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

void console_echo_off( void ) {
   struct termios s_term_current;

   tcgetattr( fileno( stdin ), &s_term_current );
   if( s_term_current.c_lflag & ECHO ) {
      s_term_current.c_lflag &= ~ECHO;
      tcsetattr( fileno( stdin ), TCSANOW, &s_term_current );
   }
}

void console_echo_on( void ) {
   struct termios s_term_current;

   tcgetattr( fileno( stdin ), &s_term_current );
   if( !(s_term_current.c_lflag & ECHO) ) {
      s_term_current.c_lflag &= ECHO;
      tcsetattr( fileno( stdin ), TCSANOW, &s_term_current );
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

