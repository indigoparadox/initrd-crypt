
#include "mount.h"

CONFIG_SCRAMBLED_STRING( gac_sys_fs_mount );
CONFIG_SCRAMBLED_STRING( gac_sys_mtype_mount );
CONFIG_SCRAMBLED_STRING( gac_sys_fs_umount );
CONFIG_SCRAMBLED_STRING( gac_sys_path_mapper );
CONFIG_SCRAMBLED_STRING( gac_sys_mpoint_root );
CONFIG_SCRAMBLED_STRING( gac_command_mdadm );

int umount_sys( void ) {
   int i_retval = 0,
      i = 0;
   char* pc_sys_fs_string = NULL,
      ** ppc_sys_fs = NULL;

   pc_sys_fs_string = config_descramble_string( &gac_sys_fs_umount );
   ppc_sys_fs = config_split_string_array( pc_sys_fs_string, NULL );

   while( NULL != ppc_sys_fs[i] ) {
      /* XXX: Handle things keeping e.g. /dev open. */
      i_retval = umount2( ppc_sys_fs[i], MNT_FORCE );
      if( i_retval ) {
         #ifdef ERRORS
         PRINTF_ERROR( "Unable to unmount %s.\n", ppc_sys_fs[i] );
         /* perror( "Unable to unmount one or more special filesystems" ); */
         #endif /* ERRORS */
         /* i_retval = ERROR_RETVAL_SYSFS_FAIL;
         goto us_cleanup; */
      }

      i++;
   }

   CONFIG_FREE_STRING_ARRAY( ppc_sys_fs );
   free( pc_sys_fs_string );

   return i_retval;
}

/* Purpose: Prepare system mounts for a minimally functioning system.         */
/* Return: 0 on success, 1 on failure.                                        */
int mount_sys( void ) {
   int i_retval = 0,
      i = 0;
   char* pc_sys_fs_string = NULL,
      ** ppc_sys_fs = NULL;
   struct stat s_dir;

   pc_sys_fs_string = config_descramble_string( &gac_sys_fs_mount );
   ppc_sys_fs = config_split_string_array( pc_sys_fs_string, NULL );

   printf( "m: %s\n", ppc_sys_fs[0] );

   while( NULL != ppc_sys_fs[i] ) {

      /* Make sure the mountpoint exists before mounting. */
      i_retval = stat( ppc_sys_fs[i], &s_dir );
      if( -1 == i_retval && ENOENT == errno ) {
         /* Create missing mountpoint. */
         mkdir( ppc_sys_fs[i], 0755 );
      } else {
         perror( "Unable to determine mountpoint status" );
         i_retval = ERROR_RETVAL_SYSFS_FAIL;
         goto ms_cleanup;
      }

      i_retval = mount( NULL, ppc_sys_fs[i], "devtmpfs", 0, "" );
      if( i_retval ) {
         #ifdef ERRORS
         perror( "Unable to mount one or more special filesystems" );
         #endif /* ERRORS */
         i_retval = ERROR_RETVAL_SYSFS_FAIL;
         goto ms_cleanup;
      }
   }

   #if 0 
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
   #endif

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
   MD_ARRAY* ps_md_arrays,
      * ps_md_array_iter;

   pc_template_mdadm = config_descramble_string( gac_command_mdadm );
   
   ps_md_arrays = config_load_md_arrays();
   ps_md_array_iter = ps_md_arrays;

   /* Iterate through the host-specific data structure and create md arrays.  */
   while( NULL != ps_md_array_iter ) {
      /* FIXME */

      #if 0
      /* XXX: Get rid of that fudge factor five. */
      i_command_mdadm_strlen = strlen( pc_template_mdadm ) + 5;
      i_command_mdadm_strlen += strlen( "/dev/" ) + 1; /* +1 for the space. */
      i_command_mdadm_strlen += strlen( ps_md_array_iter->name );

      /* Allocate a string to hold the finished command. */
      for(
         i_dev_iter = 0 ;
         ps_md_array_iter->devs_count > i_dev_iter ;
         i_dev_iter++
      ) {
         /* Add +1 for the space to precede each dev. */
         i_command_mdadm_strlen +=
            strlen( ps_md_array_iter->devs[i_dev_iter] ) + 1;
      }
      pc_command_mdadm = calloc( i_command_mdadm_strlen, sizeof( char ) );

      /* Concat each device onto the command template. */
      strcpy( pc_command_mdadm, pc_template_mdadm );
      strcat( pc_command_mdadm, "/dev/" );
      strcat( pc_command_mdadm, ps_md_array_iter->name );
      strcat( pc_command_mdadm, " " );
      for(
         i_dev_iter = 0 ;
         ps_md_array_iter->devs_count > i_dev_iter ;
         i_dev_iter++
      ) {
         strcat( pc_command_mdadm, ps_md_array_iter->devs[i_dev_iter] );
         if( ps_md_array_iter->devs_count > i_dev_iter - 1 ) {
            strcat( pc_command_mdadm, " " );
         }
      }

      i_command_mdadm_strlen += strlen( ps_md_array_iter->name );

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
            "There was a problem starting %s.\n", ps_md_array_iter->name 
         );
         #endif /* ERRORS */
         goto mm_cleanup;
      }
      #endif
   }

mm_cleanup:

   /* Perform cleanup, destroy the information structure. */
   free( pc_command_mdadm );
   free( pc_template_mdadm );
   config_free_md_arrays( ps_md_arrays );

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
      * pc_path_mapper = NULL,
      * pc_root_mountpoint = NULL;

   /* Initialize strings, etc. */
   if( regcomp( &s_regex, ".*\\-root", 0 ) ) {
      #ifdef ERRORS
      perror( "Unable to compile root regex" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_REGEX_FAIL;
      goto mpr_cleanup;
   }

   pc_path_mapper = config_descramble_string( &gac_sys_path_mapper );
   pc_root_mountpoint = config_descramble_string( &gac_sys_mpoint_root );

   /* Try to find an appropriate root device. */
   p_dev_dir = opendir( pc_path_mapper );
   if( NULL != p_dev_dir ) {
      while( (p_dev_entry = readdir( p_dev_dir )) ) {
         if( !regexec( &s_regex, p_dev_entry->d_name, 0, NULL, 0 ) ) {
            /* Create the root dev string. */
            pc_root_dev = calloc(
               strlen( pc_path_mapper ) + strlen( p_dev_entry->d_name ),
               sizeof( char )
            );
            sprintf( pc_root_dev, pc_path_mapper, p_dev_entry->d_name );
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
   free( pc_path_mapper );
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

