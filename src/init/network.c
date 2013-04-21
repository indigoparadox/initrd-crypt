
#include "config_extern.h"

#ifdef NET

#include "network.h"

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

   /* Set the IP. */
   s_ifreq.ifr_addr.sa_family = AF_INET;
   strncpy( s_ifreq.ifr_name, pc_net_if, IFNAMSIZ - 1 );
   s_addr.sin_addr.s_addr = inet_addr( pc_net_ip );
   memcpy( &s_ifreq.ifr_addr, &s_addr, sizeof( struct sockaddr ) );

   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFADDR, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl on socket" );
      #endif /* ERRORS */
      goto sn_cleanup;
   }

   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl on socket" );
      #endif /* ERRORS */
      goto sn_cleanup;
   }

   s_ifreq.ifr_flags |= IFF_UP | IFF_RUNNING;

   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl on socket" );
      #endif /* ERRORS */
      goto sn_cleanup;
   }

   #ifdef DEBUG
   /* Verify and display address. */
   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFADDR, &s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl on socket" );
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

