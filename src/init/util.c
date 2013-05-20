
#include "util.h"

/* TODO: Why do we need to include this twice? */
#include <stdarg.h>

/* Portions of this code shamelessly stolen from busybox (but changed to      *
 * fit our preferred coding style.                                            */

char* xasprintf( const char* pc_format_in, ... ) {
   va_list p;
   int i_retval;
   char *pc_string;

   va_start( p, pc_format_in );
   i_retval = vasprintf( &pc_string, pc_format_in, p );
   va_end( p );

   if( 0 > i_retval ) {
      return NULL;
   } else {
      return pc_string;
   }
}

char* last_char_is( const char *s, int c ) {
   if( s && *s ) {
      size_t sz = strlen(s) - 1;
      s += sz;
      if( (unsigned char)*s == c ) {
         return (char*)s;
      }
   }

   return NULL;
}

void fork_exec( char** ppc_command_in ) {
   int i_fork_pid;

   i_fork_pid = fork();
   if( 0 == i_fork_pid ) {
      /* This is the child process. */

      execv( ppc_command_in[0], ppc_command_in );

   } else if( 0 < i_fork_pid ) {
      /* This is the parent process. */
      return;
   }
}

int kill_pid_file( char* pc_pid_file_path_in ) {
   
   #define PID_LINE_BUFFER_SIZE 50

   int i_retval = 0,
      i_pid,
      i_pid_file;
   char ac_pid_line[PID_LINE_BUFFER_SIZE];

   PRINTF_DEBUG( "Stopping %s...\n", pc_pid_file_path_in );
   i_pid_file = open( pc_pid_file_path_in, O_RDONLY );
   if( 0 > i_pid_file ) {
      #ifdef ERRORS
      perror( "Unable to open pid file" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_EXEC_FAIL;
      goto kpf_cleanup;
   }

   PRINTF_DEBUG( "Reading %s...\n", pc_pid_file_path_in );
   if(
      0 > read( i_pid_file, ac_pid_line, PID_LINE_BUFFER_SIZE )
   ) {
      #ifdef ERRORS
      perror( "Unable to read from pid file" );
      #endif /* ERRORS */
      goto kpf_cleanup;
   }
   i_pid = atoi( ac_pid_line );

   PRINTF_DEBUG( "Found pid: %d, killing...\n", i_pid );

   ERROR_PERROR( 
      kill( i_pid, SIGTERM ),
      i_retval,
      ERROR_RETVAL_EXEC_FAIL,
      kpf_cleanup,
      "Unable to stop process\n"
   );

kpf_cleanup:

   return i_retval;
}

