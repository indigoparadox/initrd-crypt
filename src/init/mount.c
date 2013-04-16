
#include "mount.h"

/* Purpose: Prepare system mounts for a minimally functioning system.         */
/* Return: 0 on success, 1 on failure.                                        */
int mount_sys( BOOL b_umount_in ) {
   int i_retval = 0;

   if( b_umount_in ) {
      if(
         umount2( "/sys", MNT_FORCE ) ||
         umount2( "/proc", MNT_FORCE ) ||
         umount2( "/dev/pts", MNT_FORCE ) ||
         umount2( "/dev", MNT_FORCE )
      ) {
         #ifdef ERRORS
         perror( "Unable to unmount one or more special filesystems" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_SYSFS_FAIL;
         goto ms_cleanup;
      }
   } else {
      if( mount( NULL, "/dev", "devtmpfs", 0, "" ) ) {
         #ifdef ERRORS
         perror( "Unable to mount one or more special filesystems" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_SYSFS_FAIL;
         goto ms_cleanup;
      }

      mkdir( "dev/pts", 0755 );

      if(
         mount( "devpts", "/dev/pts", "devpts", 0, "" ) ||
         mount( NULL, "/proc", "proc", 0, "" ) ||
         mount( NULL, "/sys", "sysfs", 0, "" )
      ) {
         #ifdef ERRORS
         perror( "Unable to mount one or more special filesystems" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_SYSFS_FAIL;
         goto ms_cleanup;
      } else {
         if(
            system( "/sbin/vgscan --mknodes" ) ||
            system( "/sbin/vgchange -a ay" )
         ) {
            #ifdef ERRORS
            perror( "Unable to start LVM" );
            #endif /* ERRORS */
            i_retval = ERROR_RETVAL_LVM_FAIL;
            goto ms_cleanup;
         }
      }
   }

ms_cleanup:

   return i_retval;
}

/* Purpose: Setup /dev/md devices if any exist.                               */
/* Return: 0 on success, 1 on failure.                                        */
int mount_mds( void ) {
   int i_md_iter,
      i_dev_iter,
      i_md_count,
      i_retval = 0,
      i, j;
   MD_ARRAY* ap_md_arrays;
   
   i_md_count = host_md_arrays( &ap_md_arrays );

   /* Iterate through the host-specific data structure and create md arrays.  */
   for( i_md_iter = 0 ; i_md_count > i_md_iter ; i_md_iter++ ) {
      /* printf( "%s\n", bdata( ap_md_arrays[i_md_iter].name ) ); */
      for(
         i_dev_iter = 0 ;
         ap_md_arrays[i_md_iter].devs_count > i_dev_iter ;
         i_dev_iter++
      ) {
         /* FIXME: Actually perform array creation. */
         /* printf(
            "%s\n", ap_md_arrays[i_md_iter].devs[i_dev_iter]
         ); */
      }
   }

   /* Perform cleanup, destroy the information structure. */
   HOST_FREE_MD_ARRAYS( ap_md_arrays );

   /* FIXME: Abort if there's a problem creating arrays. */
   return i_retval;
}

/* Purpose: Attempt to mount root filesystem.                                 */
/* Return: 0 on success, 1 on failure.                                        */
int mount_probe_root( void ) {
   DIR* p_dev_dir;
   struct dirent* p_dev_entry;
   regex_t s_regex;
   int i_retval = 0;
   char* pc_root_dev = NULL,
      * pc_mapper_path = NULL,
      * pc_root_mountpoint = NULL;

   /* Initialize strings, etc. */
   if( regcomp( &s_regex, ".*\\-root", 0 ) ) {
      #ifdef ERRORS
      perror( "Unable to compile root search" );
      #endif /* ERRORS */
      goto mpr_cleanup;
   }

   pc_mapper_path = config_mapper_path();
   pc_root_mountpoint = config_root_mountpoint();

   /* Try to find an appropriate root device. */
   p_dev_dir = opendir( "/dev/mapper" );
   if( NULL != p_dev_dir ) {
      while( (p_dev_entry = readdir( p_dev_dir )) ) {
         if( !regexec( &s_regex, p_dev_entry->d_name, 0, NULL, 0 ) ) {
            /* Create the root dev string. */
            pc_root_dev = calloc(
               strlen( pc_mapper_path ) + strlen( p_dev_entry->d_name ),
               sizeof( char )
            );
            sprintf( pc_root_dev, pc_mapper_path, p_dev_entry->d_name );
            break;
         }
      }
      closedir( p_dev_dir );
   } else {
      #ifdef ERRORS
      perror( "Unable to open /dev/mapper" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_MAPPER_FAIL;
      goto mpr_cleanup;
   }

   /* Attempt to mount the selected root device. */
   if( mount( pc_root_dev, pc_root_mountpoint, "ext3", MS_RDONLY, "" ) ) {
      #ifdef ERRORS
      perror( "Unable to mount root device" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_ROOT_FAIL;
      goto mpr_cleanup;
   }

mpr_cleanup:

   /* Cleanup. */
   regfree( &s_regex );
   free( pc_root_dev );
   free( pc_mapper_path );
   free( pc_root_mountpoint );

   return i_retval;
}

/* Purpose: Mount /usr before boot for compatbility with newer systems.       */
/* Return: 0 on success, 1 on failure.                                        */
int mount_probe_usr( void ) {
   /* TODO: Open fstab and see if a line exists for /usr. Mount if applicable.
    */
   return -1;
}

