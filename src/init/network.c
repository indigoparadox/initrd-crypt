
#include "config_extern.h"

#include "network.h"

#ifdef SERIAL
int gi_serial_child_pid = 0;
int gi_serial_port = 0;
extern char* gpc_serial_listen;
#endif /* SERIAL */

#ifdef NET

int setup_network( void ) {
   int i_socket,
      i_retval;
   struct ifreq s_ifreq;
   struct sockaddr_in s_addr;
   char* pc_net_if = NULL,
      * pc_net_ip = NULL;

   /* Initialize. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   memset( &s_addr, '\0', sizeof( struct sockaddr_in ) );
   pc_net_if = config_descramble_string( gac_net_if, gai_net_if );
   pc_net_ip = config_descramble_string( gac_net_ip, gai_net_ip );

   /* Open a socket. */
   if( 0 > (i_socket = socket( AF_INET, SOCK_DGRAM, 0 )) ) {
      #ifdef ERRORS
      perror( "Error opening network socket" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_NET_FAIL;
      goto sn_cleanup;
   }

   /* Bring the interface up. */
   strncpy( s_ifreq.ifr_name, pc_net_if, IFNAMSIZ - 1 );

   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCGIFFLAGS on socket" );
      #endif /* ERRORS */
      goto sn_cleanup;
   }

   s_ifreq.ifr_flags |= IFF_UP | IFF_RUNNING;

   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFFLAGS on socket" );
      #endif /* ERRORS */
      goto sn_cleanup;
   }

   /* Set the IP. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   memset( &s_addr, '\0', sizeof( struct sockaddr_in ) );
   strncpy( s_ifreq.ifr_name, pc_net_if, IFNAMSIZ - 1 );
   s_addr.sin_addr.s_addr = inet_addr( pc_net_ip );
   s_addr.sin_family = AF_INET;
   s_addr.sin_port = 0;
   memcpy( &s_ifreq.ifr_addr, &s_addr, sizeof( struct sockaddr ) );

   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFADDR, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFADDR on socket" );
      #endif /* ERRORS */
      goto sn_cleanup;
   }

   #ifdef DEBUG
   /* Verify and display address. */
   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFADDR, &s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCGIFADDR on socket" );
      #endif /* ERRORS */
      goto sn_cleanup;
   }

   printf(
      "Network: %s %s\n",
      s_ifreq.ifr_name,
      inet_ntoa( ((struct sockaddr_in*)&s_ifreq.ifr_addr)->sin_addr )
   );
   #endif /* DEBUG */
   
sn_cleanup:

   close( i_socket );

   free( pc_net_if );
   free( pc_net_ip );

   return i_retval;
}

#endif /* NET */

#ifdef SERIAL

int setup_serial( void ) {
   int i_retval = 0;
   char* pc_prompt_argv_string,
      ** ppc_prompt_argv;

   pc_prompt_argv_string = config_descramble_string(
      gac_command_serial, gai_command_serial
   );
   ppc_prompt_argv = config_split_string_array( pc_prompt_argv_string );

   if( NULL != gpc_serial_listen ) {
      /* A serial port was specified to listen on, so open it up. */
      gi_serial_port = open( gpc_serial_listen, O_RDWR | O_NOCTTY | O_NDELAY );
      if( 0 > gi_serial_port ) {
         #ifdef ERRORS
         perror( "Unable to open serial port" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_SERIAL_FAIL;
         goto ss_cleanup;
      } else {
         fcntl( gi_serial_port, F_SETFL, FNDELAY );
         printf( "serial: %d\n", gi_serial_port );
      }
   } else if( 1 == getpid() ) {
      /* We're the parent, starting serial prompts. */

      /* For now, just open up a serial port and launch a prompt process on   *
       * it.                                                                  */
      gi_serial_child_pid = fork();
      if( 0 == gi_serial_child_pid ) {
         /* We're deaf/dumb on the main terminal. */
         /*close( fileno( stdin ) );
         close( fileno( stdout ) );
         close( fileno( stderr ) );*/

         /* This is the child process. Start the prompt. */
         execv( ppc_prompt_argv[0], ppc_prompt_argv );
      }
   }

ss_cleanup:

   config_free_string_array( ppc_prompt_argv );

   return i_retval;
}

int stop_serial( void ) {
   /* FIXME */
   return 0;
}

#endif /* SERIAL */

