
#include "host.h"

char gi_skey[] = {::SKEY::};

#define HOST_MD_COUNT 2
int host_md_count( void ) {
   return HOST_MD_COUNT;
}

#define HOST_MD_DEVS_PER 2
int host_md_devs_per( void ) {
   return HOST_MD_DEVS_PER;
}

int host_md_arrays( MD_ARRAY** md_arrays ) {
   int i, j;

   char aac_dev_md_names[HOST_MD_COUNT][HOST_MAX_MD_NAME_LEN] = {
      bfromcstr( "md1" ),
      bfromcstr( "md2" ),
   },
   aac_dev_md_devs[HOST_MD_COUNT][HOST_MD_DEVS_PER][HOST_MAX_MD_DEV_LEN] = {
      {
         bfromcstr( "/dev/sda1" ),
         bfromcstr( "/dev/sdb1" ),
      },
      {
         bfromcstr( "/dev/sda2" ),
         bfromcstr( "/dev/sdb2" ),
      },
   };

   HOST_PREPARE_MD_ARRAYS();

   return HOST_MD_COUNT;
};


#define HOST_LVOL_COUNT 1
int host_lvol_count( void ) {
   return HOST_LVOL_COUNT;
}

int host_lvols( LVOL** lvols ) {
   int i;
   char aac_lvol_names[HOST_LVOL_COUNT][2][HOST_MAX_LVOL_ID_LEN] = {
      {
         bfromcstr( "crypt-root" ),
         bfromcstr( "/dev/md2" ),
      },
   };

   HOST_PREPARE_LVOLS();

   return HOST_LVOL_COUNT;
}

int host_max_attempts( void ) {
   return 4;
}

char* host_net_ipv4_if( void ) {
   char ac_if_dev[] = bfromcstr( "eth0" );
   return descramble_create_string( ac_if_dev, gi_skey );
}

char* host_net_ipv4_ip( void ) {
   char ac_ip_addr[] = bfromcstr( "192.168.250.10" );
   return descramble_create_string( ac_ip_addr, gi_skey );
}

char* host_ssh_port( void ) {
   char ac_ssh_port[] = bfromcstr( "44010" );
   return descramble_create_string( ac_ssh_port, gi_skey );
}

