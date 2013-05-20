
#include "config_extern.h"

#include "network.h"

#ifdef NET

int network_start_ssh( void ) {
   int i_retval = 0;
   char* pc_command_ssh_string = NULL,
      * pc_command_ssh = NULL;

   pc_command_ssh_string = config_descramble_string(
      gac_command_ssh, gai_command_ssh
   );

   pc_command_ssh = xasprintf( pc_command_ssh_string, SSH_PORT );

   /* Launch the SSH daemon. */
   PRINTF_DEBUG( "Starting SSH server...\n" );
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

   PRINTF_DEBUG( "SSH server started on port %s.\n", SSH_PORT );

nss_cleanup:
   
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

   PRINTF_DEBUG( "Stopping SSH server...\n" );
   i_ssh_pid_file = open( pc_ssh_pid_path, O_RDONLY );
   if( 0 > i_ssh_pid_file ) {
      #ifdef ERRORS
      perror( "Unable to open SSH server PID file" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_SSH_FAIL;
      goto nxs_cleanup;
   }

   PRINTF_DEBUG( "Reading SSH server PID...\n" );
   if(
      0 > read( i_ssh_pid_file, ac_ssh_pid_line, SSH_PID_LINE_BUFFER_SIZE )
   ) {
      #ifdef ERRORS
      perror( "Unable to read from SSH server PID file" );
      #endif /* ERRORS */
      goto nxs_cleanup;
   }
   i_ssh_pid = atoi( ac_ssh_pid_line );

   PRINTF_DEBUG( "SSH PID found: %d\n", i_ssh_pid );

   PRINTF_DEBUG( "Killing SSH server...\n" );
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

/* Parameters:                                                                *
 *    i_up_in - 0 to bring the interface down, anything else to bring it up.  */
int toggle_network_interface( char* pc_if_name_in, int i_up_in ) {
   int i_retval = 0,
      i_socket;
   struct ifreq s_ifreq;

   /* Initialize. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   NETWORK_OPEN_SOCKET( i_socket, sni_cleanup );

   strncpy( s_ifreq.ifr_name, pc_if_name_in, IFNAMSIZ - 1 );
   PRINTF_DEBUG( "Getting %s flags...\n", pc_if_name_in );
   if( 0 > (i_retval = ioctl( i_socket, SIOCGIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCGIFFLAGS on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sni_cleanup;
   }
   if( !i_up_in ) {
      s_ifreq.ifr_flags &= ~IFF_UP & ~IFF_RUNNING;
      PRINTF_DEBUG( "Bringing down %s...\n", pc_if_name_in );
   } else {
      s_ifreq.ifr_flags |= IFF_UP | IFF_RUNNING;
      PRINTF_DEBUG( "Bringing up %s...\n", pc_if_name_in );
   }
   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFFLAGS, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFFLAGS on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sni_cleanup;
   }

sni_cleanup:

   close( i_socket );

   return i_retval;
}

int setup_network( void ) {
   int i_socket,
      i_retval = 0;
   struct ifreq s_ifreq;
   struct sockaddr_in s_addr;
   struct rtentry s_route;
   char* pc_net_if = NULL;
   #ifdef DHCP
   char* pc_command_dhcp_string = NULL,
      * pc_command_dhcp = NULL;
   #else
   char* pc_net_ip = NULL;
   #endif /* DHCP */
   #ifdef VLAN
   char* pc_vlan_if = NULL;
   struct vlan_ioctl_args s_vlreq;
   #endif /* VLAN */

   /* Initialize. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   memset( &s_route, '\0', sizeof( struct rtentry ) );
   memset( &s_addr, '\0', sizeof( struct sockaddr_in ) );
   pc_net_if = config_descramble_string( gac_net_if, gai_net_if );
   #ifdef DHCP
   pc_command_dhcp_string = config_descramble_string(
      gac_command_dhcp, gai_command_dhcp
   );
   #else
   pc_net_ip = config_descramble_string( gac_net_ip, gai_net_ip );
   #endif /* DHCP */

   #ifdef VLAN
   pc_vlan_if = config_descramble_string( gac_net_vlan_if, gai_net_vlan_if );
   #endif /* VLAN */

   /* Open a socket. */
   PRINTF_DEBUG( "Opening socket...\n" );
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

   PRINTF_DEBUG( "Starting VLANs...\n" );
   if( 0 > ioctl( i_socket, SIOCSIFVLAN, & s_vlreq) ) {
      #ifdef ERRORS
      perror( "Error executing SIOCSIFVLAN on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_VLAN_FAIL;
      goto sn_cleanup;
   }

   /* Bring the VLAN parent interface up. */
   toggle_network_interface( pc_vlan_if, 1 );

   /* Clean up after the parent interface. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   #endif /* VLAN */

   /* Bring localhost interface up. */
   toggle_network_interface( "lo", 1 );

   /* Bring the interface up. */
   toggle_network_interface( pc_net_if, 1 );

   #ifdef DHCP
   #ifndef ERRORS
   console_hide();
   #endif /* ERRORS */
   pc_command_dhcp = xasprintf( pc_command_dhcp_string, pc_net_if );
   ERROR_PRINTF(
      system( pc_command_dhcp ),
      i_retval,
      ERROR_RETVAL_NET_FAIL,
      sn_cleanup,
      "Unable to start DHCP.\n"
   );
   #ifndef ERRORS
   console_show();
   #endif /* ERRORS */
   #else
   /* Set the IP. */
   memset( &s_ifreq, '\0', sizeof( struct ifreq ) );
   memset( &s_addr, '\0', sizeof( struct sockaddr_in ) );
   strncpy( s_ifreq.ifr_name, pc_net_if, IFNAMSIZ - 1 );
   s_addr.sin_addr.s_addr = inet_addr( pc_net_ip );
   s_addr.sin_family = AF_INET;
   s_addr.sin_port = 0;
   memcpy( &s_ifreq.ifr_addr, &s_addr, sizeof( struct sockaddr ) );

   PRINTF_DEBUG( "Setting IP address...\n" );
   if( 0 > (i_retval = ioctl( i_socket, SIOCSIFADDR, (char*)&s_ifreq )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCSIFADDR on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sn_cleanup;
   }

   /* Set the default route. */
   s_addr.sin_addr.s_addr = inet_addr( NET_GATEWAY );
   s_addr.sin_family = AF_INET;
   s_addr.sin_port = 0;
   ((struct sockaddr_in*)&s_route.rt_dst)->sin_addr.s_addr = 0;
   ((struct sockaddr_in*)&s_route.rt_dst)->sin_family = AF_INET;
   ((struct sockaddr_in*)&s_route.rt_dst)->sin_port = 0;
   ((struct sockaddr_in*)&s_route.rt_genmask)->sin_addr.s_addr = 0;
   ((struct sockaddr_in*)&s_route.rt_genmask)->sin_family = AF_INET;
   ((struct sockaddr_in*)&s_route.rt_genmask)->sin_port = 0;
   memcpy( (void*)&s_route.rt_gateway, &s_addr, sizeof( s_addr ) );
   s_route.rt_flags = RTF_UP | RTF_GATEWAY;
   if( 0 > (i_retval = ioctl( i_socket, SIOCADDRT, &s_route )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCADDRT on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto sn_cleanup;
   }

   #ifdef DEBUG
   /* Verify and display address. */
   PRINTF_DEBUG( "Getting IP address...\n" );
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
   #endif /* DHCP */

sn_cleanup:

   //#ifndef DHCP
   close( i_socket );
   //#endif /* DHCP */

   free( pc_net_if );
   #ifdef DHCP
   free( pc_command_dhcp_string );
   free( pc_command_dhcp );
   #else
   free( pc_net_ip );
   #endif /* DHCP */

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
   struct rtentry s_route;
   char* pc_net_if = NULL;
   #ifdef DHCP
   char* pc_dhcp_pid_path_string = NULL,
      * pc_dhcp_pid_path = NULL;
   #endif /* DHCP */

   pc_net_if = config_descramble_string( gac_net_if, gai_net_if );

   #ifdef DHCP
   pc_dhcp_pid_path_string = config_descramble_string(
      gac_sys_path_dhcppid, gai_sys_path_dhcppid
   );
   #endif /* DHCP */

   /* Open a socket. */
   PRINTF_DEBUG( "Opening socket...\n" );
   if( 0 > (i_socket = socket( AF_INET, SOCK_DGRAM, 0 )) ) {
      #ifdef ERRORS
      perror( "Error opening network socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto xn_cleanup;
   }

   /* Unset the default route. */
   #if 0
   s_addr.sin_addr.s_addr = inet_addr( NET_GATEWAY );
   s_addr.sin_family = AF_INET;
   s_addr.sin_port = 0;
   ((struct sockaddr_in*)&s_route.rt_dst)->sin_addr.s_addr = 0;
   ((struct sockaddr_in*)&s_route.rt_dst)->sin_family = AF_INET;
   ((struct sockaddr_in*)&s_route.rt_dst)->sin_port = 0;
   ((struct sockaddr_in*)&s_route.rt_genmask)->sin_addr.s_addr = 0;
   ((struct sockaddr_in*)&s_route.rt_genmask)->sin_family = AF_INET;
   ((struct sockaddr_in*)&s_route.rt_genmask)->sin_port = 0;
   memcpy( (void*)&s_route.rt_gateway, &s_addr, sizeof( s_addr ) );
   s_route.rt_flags = RTF_UP | RTF_GATEWAY;
   if( 0 > (i_retval = ioctl( i_socket, SIOCDELRT, &s_route )) ) {
      #ifdef ERRORS
      perror( "Error executing ioctl SIOCDELRT on socket" );
      #endif /* ERRORS */
      i_retval |= ERROR_RETVAL_NET_FAIL;
      goto xn_cleanup;
   }
   memset( &s_addr, '\0', sizeof( struct sockaddr_in ) );
   #endif

   /* Bring loopback interface down. */
   toggle_network_interface( "lo", 0 );

   #ifdef DHCP
   pc_dhcp_pid_path = xasprintf( pc_dhcp_pid_path_string, pc_net_if );
   ERROR_PRINTF(
      kill_pid_file( pc_dhcp_pid_path ),
      i_retval,
      ERROR_RETVAL_NET_FAIL,
      xn_cleanup,
      "Unable to stop DHCP.\n"
   );
   #else
   /* Bring the interface down. */
   toggle_network_interface( pc_net_if, 0 );
   #endif /* DHCP */

   #ifdef VLAN
   /* Bring the parent interface down. */
   toggle_network_interface( pc_vlan_if, 0 );

   /* Delete the VLAN. */
   memset( &s_vlreq, '\0', sizeof( struct vlan_ioctl_args ) );

   strncpy( s_vlreq.device1, pc_net_if, IFNAMSIZ - 1 );
   s_vlreq.cmd = DEL_VLAN_CMD;

   PRINTF_DEBUG( "Deleting VLAN...\n" );
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
   
   #ifdef DHCP
   free( pc_dhcp_pid_path_string );
   free( pc_dhcp_pid_path );
   #endif /* DHCP */

   #ifdef VLAN
   free( pc_vlan_if );
   #endif /* VLAN */

   return i_retval;
}

#ifdef TOR

int network_start_tor( void ) {
   int i_retval = 0;
   char* pc_command_tor_string,
      ** ppc_command_tor;

   pc_command_tor_string = config_descramble_string(
      gac_command_tor, gai_command_tor
   );
   ppc_command_tor = config_split_string_array( pc_command_tor_string );

   #ifndef ERRORS
   console_hide();
   #endif /* ERRORS */
   fork_exec( ppc_command_tor );
   #ifndef ERRORS
   console_show();
   #endif /* ERRORS */

st_cleanup:

   config_free_string_array( ppc_command_tor );
   free( pc_command_tor_string );

   return i_retval;
}

int network_stop_tor( void ) {
   int i_retval = 0;
   char* pc_tor_pid_path = NULL;

   pc_tor_pid_path = config_descramble_string(
      gac_sys_path_torpid, gai_sys_path_torpid
   );

   ERROR_PRINTF(
      kill_pid_file( pc_tor_pid_path ),
      i_retval,
      ERROR_RETVAL_TOR_FAIL,
      xt_cleanup,
      "Unable to stop tor.\n"
   );

xt_cleanup:

   free( pc_tor_pid_path );

   return i_retval;
}

#endif /* TOR */

#endif /* NET */

