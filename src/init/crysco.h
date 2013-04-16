
#ifndef CRYSCO_H
#define CRYSCO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <wait.h>
#include <libcryptsetup.h>

#include "host.h"
#include "mount.h"
#include "error.h"

/* = Function Prototypes = */

int attempt_decrypt( char* );
int prompt_decrypt( void );

#endif /* CRYSCO_H */

