
#include "config_extern.h"

#include "network.h"

#ifdef NET

int network_start_ssh( void ) {
   int i_retval = 0;
   char* pc_command_ssh_string,
      * pc_command_ssh,
      * pc_ssh_port;

   #define SSH_PORT_INDEX 5

   pc_ssh_port = config_descramble_string( gac_ssh_port, gai_ssh_port );
   pc_command_ssh_string = config_descramble_string(
      gac_command_ssh, gai_command_ssh
   );
   /* ppc_command_ssh = config_split_string_array( pc_command_ssh_string ); */

   /* free( ppc_command_ssh[SSH_PORT_INDEX] );
   ppc_command_ssh[SSH_PORT_INDEX] = xasprintf( "%s", pc_ssh_port ); */
   pc_command_ssh = xasprintf( pc_command_ssh_string, pc_ssh_port );

   /* Launch the SSH daemon. */
   /* /sbin/dropbear -s -j -k -p $CFG_SSH_PORT >/dev/null 2>&1 */
   #ifndef ERRORS
   console_hide();
   #endif /* ERRORS */
   ERROR_PRINTF(
      system( pc_command_ssh ),
      i_retval,
      ERROR_RETVAL_SSH_FAIL,
      nss_cleanup,
      "Unable to start SSH server.\n"
   );
   #ifndef ERRORS
   console_show();
   #endif /* ERRORS */

   #ifdef DEBUG
   printf( "SSH server started on port %s.\n", pc_ssh_port );
   #endif /* DEBUG */

nss_cleanup:
   
   free( pc_ssh_port );
   free( pc_command_ssh_string );
   /* config_free_string_array( ppc_command_ssh ); */
   free( pc_command_ssh );

   return i_retval;
}

int network_signal_dyndns( void ) {
   int i_retval = 0;
   /* TODO: Update a configured dynamic DNS provider. */
   /* /bin/wget https://$CFG_DDNS_USER:$CFG_DDNS_PASS@www.dnsdynamic.org/api/?hostname=$CFG_DDNS_DOMAIN */
   return i_retval;
}

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

