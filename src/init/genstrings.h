
#ifndef GENSTRINGS_H
#define GENSTRINGS_H

#include "datatypes.h"
#include "scrambles.h"

/* = Function Prototypes = */

int command_switchroot( char*** );

char* config_action_crypt( void );
#ifdef CONSOLE
char* config_action_console( void );
#endif /* CONSOLE */

char* config_mapper_path( void );
char* config_root_mountpoint( void );

#endif /* GENSTRINGS_H */

