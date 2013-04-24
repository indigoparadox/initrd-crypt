
#include "config_extern.h"

#include "network.h"

#ifdef SERIAL
int gi_serial_port = 0;
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
   int i_retval = 0,
      i_serial_pid;
   char* pc_serial_dev,
      * pc_prompt_argv_string,
      ** ppc_prompt_argv;

   pc_serial_dev = config_descramble_string( gac_serial_dev, gai_serial_dev );
   pc_prompt_argv_string = config_descramble_string(
      gac_command_prompt, gai_command_prompt
   );
   ppc_prompt_argv = config_split_string_array( pc_prompt_argv_string );

   /* For now, just open up a serial port and launch a prompt process on it. */
   i_serial_pid = fork();
   if( 0 == i_serial_pid ) {
      /* This is the child process. */

      /* Open the serial port. */
      gi_serial_port = open( pc_serial_dev, O_RDWR | O_NOCTTY | O_NDELAY );
      if( 0 > gi_serial_port ) {
         #ifdef ERRORS
         perror( "Unable to open serial port" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_SERIAL_FAIL;
         goto ss_cleanup;
      } else {
         fcntl( gi_serial_port, F_SETFL, FNDELAY );
      }

      /* Close existing stdin/stdout and attach them to the parent's pipes. */
      close( STDIN_FILENO );
      dup2( gi_serial_port, STDIN_FILENO );
      dup2( gi_serial_port, STDERR_FILENO );

      /* Start the prompt. */
      execv( ppc_prompt_argv[0], ppc_prompt_argv );
   }

ss_cleanup:

   free( pc_serial_dev );
   config_free_string_array( ppc_prompt_argv );

   return i_retval;
}

void stop_serial( void ) {

}

#endif /* SERIAL */

