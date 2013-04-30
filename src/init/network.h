
#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#ifdef NET
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#endif /* NET */

#include "config.h"
#include "error.h"
#include "util.h"

/* = Function Prototypes = */

#ifdef NET
int network_start_ssh( void );
int network_stop_ssh( void );
int network_signal_dyndns( void );
int setup_network( void );
int stop_network( void );
#endif /* NET */

#endif /* NETWORK_H */

