
#ifndef GENSTRINGS_H
#define GENSTRINGS_H

#include "datatypes.h"
#include "scrambles.h"

#define CONFIG_STRINGLIST_FREE( stringlist, stringlist_len ) \
   for( i = 0 ; stringlist_len > i ; i++ ) { \
      free( stringlist[i] ); \
   } \
   free( stringlist );

/* = Function Prototypes = */

int command_switchroot( char*** );
char* command_mdadm( void );

char* config_action_crypt( void );
#ifdef CONSOLE
char* config_action_console( void );
#endif /* CONSOLE */

char** config_sys_fs( void );
char* config_mapper_path( void );
char* config_root_mountpoint( void );

#endif /* GENSTRINGS_H */

