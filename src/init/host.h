
#ifndef HOSTNAME_H
#define HOSTNAME_H

#include "datatypes.h"
#include "scrambles.h"

#define HOST_MAX_MD_NAME_LEN 4
#define HOST_MAX_MD_DEV_LEN 10
#define HOST_MAX_LVOL_ID_LEN 15

/* = Macros = */

#define HOST_PREPARE_MD_ARRAYS() \
   *md_arrays = calloc( host_md_count(), sizeof( MD_ARRAY ) ); \
   for( i = 0 ; host_md_count() > i ; i++ ) { \
      (*md_arrays)[i].name = descramble_create_string( \
         aac_dev_md_names[i], gi_skey \
      ); \
      /* TODO: Allow for md devices with varying count of devs. */ \
      (*md_arrays)[i].devs_count = host_md_devs_per(); \
      (*md_arrays)[i].devs = calloc( host_md_devs_per(), sizeof( char* ) ); \
      for( j = 0 ; host_md_devs_per() > j ; j++ ) { \
         (*md_arrays)[i].devs[j] = \
            descramble_create_string( aac_dev_md_devs[i][j], gi_skey ); \
      } \
   }

#define HOST_FREE_MD_ARRAYS( md_array_target ) \
   for( i = 0 ; host_md_count() > i ; i++ ) { \
      for( j = 0 ; host_md_devs_per() > j ; j++ ) { \
         free( md_array_target[i].devs[j] ); \
      } \
      free( md_array_target[i].name ); \
      free( md_array_target[i].devs ); \
   } \
   free( md_array_target );

#define HOST_PREPARE_LVOLS() \
   *lvols = calloc( HOST_LVOL_COUNT, sizeof( LVOL ) ); \
   for( i = 0 ; host_lvol_count() > i ; i++ ) { \
      (*lvols)[i].name = descramble_create_string( \
         aac_lvol_names[i][0], gi_skey \
      ); \
      (*lvols)[i].dev = descramble_create_string( \
         aac_lvol_names[i][1], gi_skey \
      ); \
   }

#define HOST_FREE_LVOLS( lvols_target ) \
   for( i = 0 ; host_lvol_count() > i ; i++ ) { \
      free( lvols_target[i].name ); \
      free( lvols_target[i].dev ); \
   } \
   free( lvols_target );

/* = Function Prototypes = */

int host_md_count( void );
int host_md_devs_per( void );
int host_md_arrays( MD_ARRAY** );
int host_lvol_count( void );
int host_lvols( LVOL** );
int host_max_attempts( void );
char* host_net_ipv4_if( void );
char* host_net_ipv4_ip( void );
char* host_ssh_port( void );

#endif /* HOSTNAME_H */

