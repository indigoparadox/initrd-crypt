
#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef NET
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <net/route.h>
#ifdef VLAN
#include <linux/if_vlan.h>
#include <linux/sockios.h>
#endif /* VLAN */
#endif /* NET */

#include "config.h"
#include "error.h"
#include "util.h"
#include "console.h"

/* = Macros = */

#ifdef ERRORS
#define NETWORK_OPEN_SOCKET( socketd, golabel ) \
   if( 0 > (socketd = socket( AF_INET, SOCK_DGRAM, 0 )) ) { \
      perror( "Error opening network socket" ); \
      i_retval |= ERROR_RETVAL_NET_FAIL; \
      goto golabel; \
   }
#else
#define NETWORK_OPEN_SOCKET( socketd, golabel ) \
   if( 0 > (socketd = socket( AF_INET, SOCK_DGRAM, 0 )) ) { \
      i_retval |= ERROR_RETVAL_NET_FAIL; \
      goto golabel; \
   }
#endif /* ERRORS */

/* = Function Prototypes = */

#ifdef NET
int network_start_ssh( void );
int network_stop_ssh( void );
int network_signal_dyndns( void );
int setup_network( void );
int stop_network( void );

#ifdef TOR
int setup_tor( void );
int stop_tor( void );
#endif /* TOR */

#endif /* NET */

#endif /* NETWORK_H */

