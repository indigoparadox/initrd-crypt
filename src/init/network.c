
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
   PRINTF_DEBUG( "Starting SSH server..." );
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

int network_stop_ssh( void ) {

   /* TODO: Define the constant centrally/use another constant. */
   #define SSH_PID_LINE_BUFFER_SIZE 50

   int i_retval = 0,
      i_ssh_pid,
      i_ssh_pid_file;
   char* pc_ssh_pid_path,
      ac_ssh_pid_line[SSH_PID_LINE_BUFFER_SIZE];

   pc_ssh_pid_path = config_descramble_string( 
      gac_sys_path_sshpid,
      gai_sys_path_sshpid
   );

   PRINTF_DEBUG( "Stopping SSH server..." );
   i_ssh_pid_file = open( pc_ssh_pid_path, O_RDONLY );
   if( 0 > i_ssh_pid_file ) {
      #ifdef ERRORS
      perror( "Unable to open SSH server PID file" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_SSH_FAIL;
      goto nxs_cleanup;
   }

   PRINTF_DEBUG( "Reading SSH server PID..." );
   if(
      0 > read( i_ssh_pid_file, ac_ssh_pid_line, SSH_PID_LINE_BUFFER_SIZE )
   ) {
      #ifdef ERRORS
      perror( "Unable to read from SSH server PID file" );
      #endif /* ERRORS */
      goto nxs_cleanup;
   }
   i_ssh_pid = atoi( ac_ssh_pid_line );

   #ifdef DEBUG
   printf( "SSH PID found: %d\n", i_ssh_pid );
   #endif /* DEBUG */

   PRINTF_DEBUG( "Killing SSH server..." );
   ERROR_PERROR( 
      kill( i_ssh_pid, SIGTERM ),
      i_retval,
      ERROR_RETVAL_SSH_FAIL,
      nxs_cleanup,
      "Unable to stop SSH daemon\n"
   );

nxs_cleanup:

   close( i_ssh_pid_file );
   free( pc_ssh_pid_path );

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
      i_retval = 0;
   struct ifreq s_ifreq;
   struct sockaddr_in s_addr;
   char* pc_net_if = NULL,
      * pc_net_ip = NULL;
   #ifdef VLAN
   char* pc_vlan_if = NULL;
   struct vlan_ioctl_args s_vlreq;
   #endif /* VLAN */

   /* Initialize. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   memset( &s_addr, '\0', sizeof( struct sockaddr_in ) );
   pc_net_if = config_descramble_string( gac_net_if, gai_net_if );
   pc_net_ip = config_descramble_string( gac_net_ip, gai_net_ip );

   #ifdef VLAN
   pc_vlan_if = config_descramble_string( gac_net_vlan_if, gai_net_vlan_if );
   #endif /* VLAN */

   /* Open a socket. */
   PRINTF_DEBUG( "Opening socket..." );
   if( 0 > (i_socket = socket( AF_INET, SOCK_DGRAM, 0 )) ) {
      #ifdef ERRORS
      perror( "Error opening network socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sn_cleanup;
   }

   #ifdef VLAN
   /* Setup the VLAN. */
   memset( &s_vlreq, '\0', sizeof( struct vlan_ioctl_args ) );

   strncpy( s_vlreq.device1, pc_vlan_if, IFNAMSIZ - 1 );
   s_vlreq.u.VID = VLAN_VID;
   s_vlreq.cmd = ADD_VLAN_CMD;

   PRINTF_DEBUG( "Starting VLANs..." );
   if( 0 > ioctl( i_socket, SIOCSIFVLAN, & s_vlreq) ) {
      #ifdef ERRORS
      perror( "Error executing SIOCSIFVLAN on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_VLAN_FAIL;
      goto sn_cleanup;
   }

   /* Bring the VLAN parent interface up. */
   strncpy( s_ifreq.ifr_name, pc_vlan_if, IFNAMSIZ - 1 );
   PRINTF_DEBUG( "Getting parent interface flags..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCGIFFLAGS on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sn_cleanup;
   }
   s_ifreq.ifr_flags |= IFF_UP | IFF_RUNNING;
   PRINTF_DEBUG( "Bringing up parent interface..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFFLAGS on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sn_cleanup;
   }
   
   /* Clean up after the parent interface. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   #endif /* VLAN */

   /* Bring the interface up. */
   strncpy( s_ifreq.ifr_name, pc_net_if, IFNAMSIZ - 1 );
   PRINTF_DEBUG( "Getting interface flags..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCGIFFLAGS on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sn_cleanup;
   }
   s_ifreq.ifr_flags |= IFF_UP | IFF_RUNNING;
   PRINTF_DEBUG( "Bringing up interface..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFFLAGS on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
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

   PRINTF_DEBUG( "Setting IP address..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFADDR, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFADDR on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sn_cleanup;
   }

   #ifdef DEBUG
   /* Verify and display address. */
   PRINTF_DEBUG( "Getting IP address..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFADDR, &s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCGIFADDR on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
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

   #ifdef VLAN
   free( pc_vlan_if );
   #endif /* VLAN */

   return i_retval;
}

int stop_network( void ) {
   int i_socket,
      i_retval = 0;
   struct ifreq s_ifreq;
   struct sockaddr_in s_addr;
   char* pc_net_if = NULL,
      * pc_net_ip = NULL;
   #ifdef VLAN
   char* pc_vlan_if = NULL;
   struct vlan_ioctl_args s_vlreq;
   #endif /* VLAN */
   
   #ifdef VLAN
   pc_vlan_if = config_descramble_string( gac_net_vlan_if, gai_net_vlan_if );
   #endif /* VLAN */

   /* Initialize. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   memset( &s_addr, '\0', sizeof( struct sockaddr_in ) );
   pc_net_if = config_descramble_string( gac_net_if, gai_net_if );
   pc_net_ip = config_descramble_string( gac_net_ip, gai_net_ip );

   /* Open a socket. */
   PRINTF_DEBUG( "Opening socket..." );
   if( 0 > (i_socket = socket( AF_INET, SOCK_DGRAM, 0 )) ) {
      #ifdef ERRORS
      perror( "Error opening network socket" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_NET_FAIL;
      goto xn_cleanup;
   }

   /* Bring the interface down. */
   strncpy( s_ifreq.ifr_name, pc_net_if, IFNAMSIZ - 1 );
   PRINTF_DEBUG( "Getting interface flags..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCGIFFLAGS on socket" );
      #endif /* ERRORS */
      goto xn_cleanup;
   }
   s_ifreq.ifr_flags &= ~IFF_UP & ~IFF_RUNNING;
   PRINTF_DEBUG( "Bringing down interface..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFFLAGS on socket" );
      #endif /* ERRORS */
      goto xn_cleanup;
   }

   #ifdef VLAN
   /* Bring the parent interface down. */
   strncpy( s_ifreq.ifr_name, pc_vlan_if, IFNAMSIZ - 1 );
   PRINTF_DEBUG( "Getting parent interface flags..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCGIFFLAGS on socket" );
      #endif /* ERRORS */
      goto xn_cleanup;
   }
   s_ifreq.ifr_flags &= ~IFF_UP & ~IFF_RUNNING;
   PRINTF_DEBUG( "Bringing down parent interface..." );
   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFFLAGS on socket" );
      #endif /* ERRORS */
      goto xn_cleanup;
   }

   /* Delete the VLAN. */
   memset( &s_vlreq, '\0', sizeof( struct vlan_ioctl_args ) );

   strncpy( s_vlreq.device1, pc_net_if, IFNAMSIZ - 1 );
   s_vlreq.cmd = DEL_VLAN_CMD;

   PRINTF_DEBUG( "Deleting VLAN..." );
   if( 0 > ioctl( i_socket, SIOCSIFVLAN, & s_vlreq) ) {
      #ifdef ERRORS
      perror( "Error executing SIOCSIFVLAN on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_VLAN_FAIL;
      goto xn_cleanup;
   }
   #endif /* VLAN */

xn_cleanup:

   close( i_socket );

   free( pc_net_if );
   free( pc_net_ip );

   #ifdef VLAN
   free( pc_vlan_if );
   #endif /* VLAN */

   return i_retval;
}

#endif /* NET */

