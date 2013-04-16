
#ifndef GENSTRINGS_H
#define GENSTRINGS_H

#include "datatypes.h"
#include "scrambles.h"

/* = Function Prototypes = */

int command_cryptsetup( char*** );

char* config_mapper_path( void );
char* config_root_mountpoint( void );

#endif /* GENSTRINGS_H */

