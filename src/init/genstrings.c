
#include "genstrings.h"

extern char gi_skey[];

int command_crypstsetup( char*** pppc_argv_in ) {
   int i;
   #define CRYPTSETUP_ARGV_COUNT 2
   char aac_cryptsetup_argv[CRYPTSETUP_ARGV_COUNT][20] = {
      bfromcstr( "/sbin/cryptsetup" ),
      bfromcstr( "luksOpen" ),
   };

   /* FIXME: Accept device arguments and copy them into argv. */

   /* Add a null terminator (+1) to the end for execv(). */
   *pppc_argv_in = calloc( CRYPTSETUP_ARGV_COUNT + 1, sizeof( char* ) );
   for( i = 0 ; CRYPTSETUP_ARGV_COUNT > i ; i++ ) {
      (*pppc_argv_in)[i] = descramble_create_string(
         aac_cryptsetup_argv[i],
         gi_skey
      );
   }

   return CRYPTSETUP_ARGV_COUNT;
}

char* config_mapper_path( void ) {
   char ac_mapper_path[] = bfromcstr( "/dev/mapper/%s" );
   return descramble_create_string( ac_mapper_path, gi_skey );
}

char* config_root_mountpoint( void ) {
   char ac_root_mountpoint[] = bfromcstr( "/mnt/root" );
   return descramble_create_string( ac_root_mountpoint, gi_skey );
}

