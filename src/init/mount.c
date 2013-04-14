
#include "mount.h"

/* Purpose: Prepare system mounts for a minimally functioning system.         */
int mount_sys( BOOL b_umount_in ) {
   if( b_umount_in ) {
      if(
         umount2( "/sys", MNT_FORCE ) ||
         umount2( "/proc", MNT_FORCE ) ||
         umount2( "/dev/pts", MNT_FORCE ) ||
         umount2( "/dev", MNT_FORCE )
      ) {
         perror( "Unable to unmount one or more special filesystems" );
         return -1;
      }
   } else {
      if( mount( NULL, "/dev", "devtmpfs", 0, "" ) ) {
         perror( "Unable to mount one or more special filesystems" );
         return -1;
      }

      mkdir( "dev/pts", 0755 );

      if(
         mount( "devpts", "/dev/pts", "devpts", 0, "" ) ||
         mount( NULL, "/proc", "proc", 0, "" ) ||
         mount( NULL, "/sys", "sysfs", 0, "" )
      ) {
         perror( "Unable to mount one or more special filesystems" );
         return -1;
      }
   }

   return 0;
}

/* Purpose: Setup /dev/md devices if any exist.                               */
int mount_mds( void ) {
   int i_md_iter,
      i_dev_iter,
      i_md_count;
   MD_ARRAY* ap_md_arrays;
   
   i_md_count = get_md_arrays( &ap_md_arrays );

   /* Iterate through the host-specific data structure and create md arrays.  */
   for( i_md_iter = 0 ; i_md_count > i_md_iter ; i_md_iter++ ) {
      /* printf( "%s\n", bdata( ap_md_arrays[i_md_iter].name ) ); */
      for(
         i_dev_iter = 0 ;
         ap_md_arrays[i_md_iter].devs->qty > i_dev_iter ;
         i_dev_iter++
      ) {
         /* FIXME: Actually perform array creation. */
         /* printf(
            "%s\n", bdata( ap_md_arrays[i_md_iter].devs->entry[i_dev_iter] )
         ); */
      }
   }

   /* Perform cleanup, destroy the information structure. */
   for( i_md_iter = 0 ; i_md_count > i_md_iter ; i_md_iter++ ) {
      bstrListDestroy( ap_md_arrays[i_md_iter].devs );
      bdestroy( ap_md_arrays[i_md_iter].name );
   }
   free( ap_md_arrays );

   /* FIXME: Abort if there's a problem creating arrays. */
   return 0;
}

/* Purpose: Mount /usr before boot for compatbility with newer systems.       */
int mount_probe_usr( void ) {
   /* TODO: Open fstab and see if a line exists for /usr. Mount if applicable.
    */
   return -1;
}

