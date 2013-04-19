
#ifndef CRYSCO_H
#define CRYSCO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <libcryptsetup.h>

#include "config.h"
#include "mount.h"
#include "error.h"

/* = Function Prototypes = */

#ifdef CONSOLE
int console( void );
#endif /* CONSOLE */
int attempt_decrypt( char* );
int prompt_decrypt( void );

#endif /* CRYSCO_H */

