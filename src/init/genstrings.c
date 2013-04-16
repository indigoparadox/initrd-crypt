
#include "genstrings.h"

extern char gi_skey[];

int command_cryptsetup( char*** pppc_argv_in ) {
   int i;
   /* CRYPTSETUP_ARGV_COUNT should only reflect the count of ACTUAL arguments *
    * specified in aac_cryptsetup_argv. The code will compensate for dev or   *
    * file args and the NULL pointer at the end.                              */
   #define CRYPTSETUP_ARGV_COUNT 2
   char aac_cryptsetup_argv[CRYPTSETUP_ARGV_COUNT][20] = {
      bfromcstr( "/sbin/cryptsetup" ),
      bfromcstr( "luksOpen" ),
   };

   /* Add null blank slots (+2) to the end for device args. */
   /* Add a null terminator (+1) to the end for execv(). */
   *pppc_argv_in = calloc( CRYPTSETUP_ARGV_COUNT + 3, sizeof( char* ) );
   for( i = 0 ; CRYPTSETUP_ARGV_COUNT > i ; i++ ) {
      (*pppc_argv_in)[i] = descramble_create_string(
         aac_cryptsetup_argv[i],
         gi_skey
      );
   }

   return CRYPTSETUP_ARGV_COUNT;
}

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

char* config_mapper_path( void ) {
   char ac_mapper_path[] = bfromcstr( "/dev/mapper/%s" );
   return descramble_create_string( ac_mapper_path, gi_skey );
}

char* config_root_mountpoint( void ) {
   char ac_root_mountpoint[] = bfromcstr( "/mnt/root" );
   return descramble_create_string( ac_root_mountpoint, gi_skey );
}

