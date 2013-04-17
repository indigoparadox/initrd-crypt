
#include "genstrings.h"

extern char gi_skey[];

int command_switchroot( char*** pppc_argv_in ) {
   int i;
   #define SWITCHROOT_ARGV_COUNT 3
   char aac_switchroot_argv[SWITCHROOT_ARGV_COUNT][20] = {
      bfromcstr( "switch_root" ),
      bfromcstr( "/mnt/root" ),
      bfromcstr( "/sbin/init" ),
   };

   /* Add a null terminator (+1) to the end for execv(). */
   *pppc_argv_in = calloc( SWITCHROOT_ARGV_COUNT + 1, sizeof( char* ) );
   for( i = 0 ; SWITCHROOT_ARGV_COUNT > i ; i++ ) {
      (*pppc_argv_in)[i] = descramble_create_string(
         aac_switchroot_argv[i],
         gi_skey
      );
   }

   return SWITCHROOT_ARGV_COUNT;
}

char* command_mdadm( void ) {
   char ac_command_mdadm[] = bfromcstr( "mdadm --assemble " );
   return descramble_create_string( ac_command_mdadm, gi_skey );
}

char* config_action_crypt( void ) {
   char ac_action_crypt[] = bfromcstr( "crypt" );
   return descramble_create_string( ac_action_crypt, gi_skey );
}

#ifdef CONSOLE
char* config_action_console( void ) {
   char ac_action_console[] = bfromcstr( "console" );
   return descramble_create_string( ac_action_console, gi_skey );
}
#endif /* CONSOLE */

/* What a mess! We should try to tidy this up with preprocessing or something *
 * later on. For now, if you add an FS, make sure to increment                *
 * CONFIG_SYS_FS_COUNT.                                                       */
#define CONFIG_SYS_FS_COUNT 4
const int cgi_config_sys_fs_count = CONFIG_SYS_FS_COUNT;
char** config_sys_fs( void ) {
   char ac_config_sysfs[CONFIG_SYS_FS_COUNT][20] = {
      bfromcstr( "/sys" ),
      bfromcstr( "/proc" ),
      bfromcstr( "/dev" ),
      bfromcstr( "/dev/pts" ),
   },
   ** ppc_config_sysfs_out = NULL;
   int i;
   
   ppc_config_sysfs_out = calloc( cgi_config_sys_fs_count, sizeof( char* ) );

   for( i = 0 ; i < cgi_config_sys_fs_count ; i++ ) {
      ppc_config_sysfs_out[i] = descramble_create_string(
         &ac_config_sysfs[i][0], gi_skey
      );
   }

   return ppc_config_sysfs_out;
}


char* config_mapper_path( void ) {
   char ac_mapper_path[] = bfromcstr( "/dev/mapper/%s" );
   return descramble_create_string( ac_mapper_path, gi_skey );
}

char* config_root_mountpoint( void ) {
   char ac_root_mountpoint[] = bfromcstr( "/mnt/root" );
   return descramble_create_string( ac_root_mountpoint, gi_skey );
}

