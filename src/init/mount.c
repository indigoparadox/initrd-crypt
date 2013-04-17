
#include "mount.h"

extern const int cgi_config_sys_fs_count;

int umount_sys( void ) {
   int i_retval = 0,
      i;
   char** ppc_sys_fs = NULL;

   ppc_sys_fs = config_sys_fs();

   for( i = cgi_config_sys_fs_count - 1 ; 0 <= i ; i-- ) {
      /* XXX: Handle things keeping e.g. /dev open. */
      i_retval = umount2( ppc_sys_fs[i], MNT_FORCE );
      if( i_retval ) {
         #ifdef ERRORS
         perror( "Unable to unmount one or more special filesystems" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_SYSFS_FAIL;
         goto us_cleanup;
      }
   }

us_cleanup:

   CONFIG_STRINGLIST_FREE( ppc_sys_fs, cgi_config_sys_fs_count );

   return i_retval;
}

/* Purpose: Prepare system mounts for a minimally functioning system.         */
/* Return: 0 on success, 1 on failure.                                        */
int mount_sys( void ) {
   int i_retval = 0;

   #if 0
      i;
   char** ppc_sys_fs = NULL;
   struct stat s_dir;

   ppc_sys_fs = config_sys_fs();

   for( i = 0 ; cgi_config_sys_fs_count > 0 ; i++ ) {
      i_retval = stat( ppc_sys_fs[i], &s_dir );
      if( -1 == i_retval && ENOENT == errno ) {
         /* Create missing mountpoint. */
         mkdir( ppc_sys_fs[i], 0755 );
      } else {
         perror( "Unable to determine mountpoint status" );
         i_retval = ERROR_RETVAL_SYSFS_FAIL;
         goto ms_cleanup;
      }

      i_retval = mount( NULL, "/dev", "devtmpfs", 0, "" ) ) {
      #ifdef ERRORS
      perror( "Unable to mount one or more special filesystems" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_SYSFS_FAIL;
      goto ms_cleanup;
   }
   #endif

   if( mount( NULL, "/dev", "devtmpfs", 0, "" ) ) {
      #ifdef ERRORS
      perror( "Unable to mount one or more special filesystems" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_SYSFS_FAIL;
      goto ms_cleanup;
   }

   mkdir( "/dev/pts", 0755 );

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
         /* TODO: Being unable to start LVM isn't always a fatal error. */
         i_retval = ERROR_RETVAL_LVM_FAIL;
         goto ms_cleanup;
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
      i_command_mdadm_strlen = 0,
      i_md_count,
      i_retval = 0,
      i_stdout_temp = 0,
      i_stderr_temp = 0,
      i_null_fd = 0,
      i, j;
   char* pc_template_mdadm = NULL,
      * pc_command_mdadm = NULL;
   MD_ARRAY* ap_md_arrays;

   pc_template_mdadm = command_mdadm();
   
   i_md_count = host_md_arrays( &ap_md_arrays );

   /* Iterate through the host-specific data structure and create md arrays.  */
   for( i_md_iter = 0 ; i_md_count > i_md_iter ; i_md_iter++ ) {
      /* printf( "%s\n", bdata( ap_md_arrays[i_md_iter].name ) ); */

      /* XXX: Get rid of that fudge factor five. */
      i_command_mdadm_strlen = strlen( pc_template_mdadm ) + 5;
      i_command_mdadm_strlen += strlen( "/dev/" ) + 1; /* +1 for the space. */
      i_command_mdadm_strlen += strlen( ap_md_arrays[i_md_iter].name );

      /* Allocate a string to hold the finished command. */
      for(
         i_dev_iter = 0 ;
         ap_md_arrays[i_md_iter].devs_count > i_dev_iter ;
         i_dev_iter++
      ) {
         /* Add +1 for the space to precede each dev. */
         i_command_mdadm_strlen +=
            strlen( ap_md_arrays[i_md_iter].devs[i_dev_iter] ) + 1;
      }
      pc_command_mdadm = calloc( i_command_mdadm_strlen, sizeof( char ) );

      /* Concat each device onto the command template. */
      strcpy( pc_command_mdadm, pc_template_mdadm );
      strcat( pc_command_mdadm, "/dev/" );
      strcat( pc_command_mdadm, ap_md_arrays[i_md_iter].name );
      strcat( pc_command_mdadm, " " );
      for(
         i_dev_iter = 0 ;
         ap_md_arrays[i_md_iter].devs_count > i_dev_iter ;
         i_dev_iter++
      ) {
         strcat( pc_command_mdadm, ap_md_arrays[i_md_iter].devs[i_dev_iter] );
         if( ap_md_arrays[i_md_iter].devs_count > i_dev_iter - 1 ) {
            strcat( pc_command_mdadm, " " );
         }
      }

      i_command_mdadm_strlen += strlen( ap_md_arrays[i_md_iter].name );

      /* Close stdout/stderr if we're squelching errors. */
      #ifndef ERRORS
      dup2( 1, i_stdout_temp );
      dup2( 2, i_stderr_temp );
      i_null_fd = open( "/dev/null", O_WRONLY );
      dup2( i_null_fd, 1 );
      dup2( i_null_fd, 2 );
      #endif /* ERRORS */

      i_retval = system( pc_command_mdadm );

      /* Restore stdout/stderr. */
      #ifndef ERRORS
      dup2( i_stdout_temp, 1 );
      dup2( i_stderr_temp, 2 );
      close( i_null_fd );
      #endif /* ERRORS */

      free( pc_command_mdadm );

      if( i_retval ) {
         #ifdef ERRORS
         PRINTF_ERROR(
            "There was a problem starting %s.\n", ap_md_arrays[i_md_iter].name 
         );
         #endif /* ERRORS */
         goto mm_cleanup;
      }
   }

mm_cleanup:

   /* Perform cleanup, destroy the information structure. */
   HOST_FREE_MD_ARRAYS( ap_md_arrays );

   return i_retval;
}

/* Purpose: Attempt to mount boot filesystem.                                 */
/* Return: 0 on success, 1 on failure.                                        */
int mount_probe_boot( void ) {
   /* TODO: Implement /mnt/boot probing for loopback/squashfs mounting. */
   /* BLOCK_PROBE_FOUND=0
   for BLK_PROBE_ITER in `ls -1 /dev/sd*`; do
      /bin/mount -o ro "$BLK_PROBE_ITER" /mnt/boot 2>/dev/null
      if [ -f "$1" ]; then
         BLOCK_PROBE_FOUND=1
      fi
   done */

   return -1;
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
      perror( "Unable to compile root regex" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_REGEX_FAIL;
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

      /* Make sure we picked up a root device. */
      if( NULL == pc_root_dev ) {
         #ifdef ERRORS
         /* TODO: Create a macro/function to print errors to stderr. */
         printf( "Unable to find root device.\n" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_ROOT_FAIL;
         goto mpr_cleanup;
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
      /* Keep trying until we find an FS that works. */
      if( mount( pc_root_dev, pc_root_mountpoint, "ext2", MS_RDONLY, "" ) ) {
         #ifdef ERRORS
         printf( "%s %s\n", pc_root_dev, pc_root_mountpoint );
         perror( "Unable to mount root device" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_ROOT_FAIL;
         goto mpr_cleanup;
      }
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
   /* FIXME */
   return 0;
}

