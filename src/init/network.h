
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
#endif /* NET */

#include "config.h"
#include "error.h"

/* = Function Prototypes = */

#ifdef NET
int setup_network( void );
void stop_network( void );
#endif /* NET */

#endif /* NETWORK_H */

