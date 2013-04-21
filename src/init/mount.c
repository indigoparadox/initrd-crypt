
#include "config_extern.h"

#include "mount.h"

/* Notes: pc_sys_fs_string format is mount_point<block_device|fs_type>        */
int umount_sys( void ) {
   int i_retval = 0;
   char* pc_sys_fs_string = NULL;
   struct string_holder* ps_sys_fs = NULL,
      * ps_sys_fs_iter = NULL,
      * ps_sys_fs_prev = NULL;

   /* Unpack the FS information. */
   pc_sys_fs_string = config_descramble_string(
      gac_sys_fs_mount,
      gai_sys_fs_mount
   );
   ps_sys_fs = config_split_string_holders( pc_sys_fs_string );
   ps_sys_fs_iter = ps_sys_fs;

   /* Find the last device in the list. */
   while( NULL != ps_sys_fs_iter ) {
      ps_sys_fs_prev = ps_sys_fs_iter;
      ps_sys_fs_iter = ps_sys_fs_iter->next;
   }
   ps_sys_fs_iter = ps_sys_fs_prev;

   /* Dismount the devices in the reverse order in which they were mounted. */
   while( 1 ) {
      /* TODO: Handle things keeping e.g. /dev open. */
      i_retval = umount2( ps_sys_fs_iter->name, MNT_FORCE );
      if( i_retval ) {
         #ifdef ERRORS
         PRINTF_ERROR( "Unable to unmount %s.\n", ps_sys_fs_iter->name );
         /* perror( "Unable to unmount one or more special filesystems" ); */
         #endif /* ERRORS */
      }

      /* Iterate. */
      if( ps_sys_fs == ps_sys_fs_iter ) {
         /* We're at the head, so quit. */
         break;
      } else {
         /* Find the previous device in the list. */
         ps_sys_fs_prev = ps_sys_fs_iter;
         ps_sys_fs_iter = ps_sys_fs;
         while( ps_sys_fs_iter->next != ps_sys_fs_prev ) {
            ps_sys_fs_iter = ps_sys_fs_iter->next;
         }
      }
   }

   return i_retval;
}

/* Purpose: Prepare system mounts for a minimally functioning system.         */
/* Return: 0 on success, 1 on failure.                                        */
/* Notes: pc_sys_fs_string format is mount_point<block_device|fs_type>        */
int mount_sys( void ) {
   int i_retval = 0;
   char* pc_sys_fs_string = NULL;
   struct string_holder* ps_sys_fs = NULL,
      * ps_sys_fs_iter = NULL;
   struct stat s_dir;

   /* Unpack the FS information. */
   pc_sys_fs_string = config_descramble_string(
      gac_sys_fs_mount,
      gai_sys_fs_mount
   );
   ps_sys_fs = config_split_string_holders( pc_sys_fs_string );
   ps_sys_fs_iter = ps_sys_fs;

   while( NULL != ps_sys_fs_iter ) {

      /* Make sure the mountpoint exists before mounting. */
      i_retval = stat( ps_sys_fs_iter->name, &s_dir );
      if( -1 == i_retval && ENOENT == errno ) {
         /* Create missing mountpoint. */
         mkdir( ps_sys_fs_iter->name, 0755 );
      }

      /* Perform the actual mount. */
      if( !strcmp( "none", ps_sys_fs_iter->strings[0] ) ) {
         /* No block device specified. */
         i_retval = mount(
            NULL,
            ps_sys_fs_iter->name,
            ps_sys_fs_iter->strings[1],
            0,
            ""
         );
      } else {
         i_retval = mount(
            ps_sys_fs_iter->strings[0],
            ps_sys_fs_iter->name,
            ps_sys_fs_iter->strings[1],
            0,
            ""
         );
      }

      if( i_retval ) {
         #ifdef ERRORS
         perror( "Unable to mount one or more special filesystems" );
         #endif /* ERRORS */
      }

      ps_sys_fs_iter = ps_sys_fs_iter->next;
   }

   free( pc_sys_fs_string );
   config_free_string_holders( ps_sys_fs );

   return i_retval;
}

/* Purpose: Setup /dev/md devices if any exist.                               */
/* Return: 0 on success, 1 on failure.                                        */
int mount_mds( void ) {
   int i_command_mdadm_strlen = 0,
      i_retval = 0,
      i_stdout_temp = 0,
      i_stderr_temp = 0,
      i_null_fd = 0,
      i;
   char* pc_template_mdadm = NULL,
      /* * pc_command_mdadm = NULL, */
      * pc_md_arrays = NULL,
      /* FIXME: Give this constant a meaningful name. Or work out effective   *
      *        dynamic allocation.                                            */
      ac_command_mdadm[255];
   struct string_holder* ps_md_arrays,
      * ps_md_array_iter;

   pc_template_mdadm = config_descramble_string(
      gac_command_mdadm,
      gai_command_mdadm
   );
   
   pc_md_arrays = config_descramble_string( gac_md_arrays, gai_md_arrays );
   ps_md_arrays = config_split_string_holders( pc_md_arrays );
   ps_md_array_iter = ps_md_arrays;

   /* Iterate through the host-specific data structure and create md arrays.  */
   while( NULL != ps_md_array_iter ) {
      i_command_mdadm_strlen += strlen( "/dev/" ) + 1; /* +1 for the space. */
      i_command_mdadm_strlen += strlen( ps_md_array_iter->name );

      #if 0
      /* Allocate a string to hold the finished command. */
      i = 0;
      while( NULL != ps_md_array_iter->strings[i] ) {
         
         /* Add +1 for the space to precede each dev. */
         i_command_mdadm_strlen += strlen( ps_md_array_iter->strings[i] ) + 1;

         i++;
      }
      pc_command_mdadm = calloc( i_command_mdadm_strlen, sizeof( char ) );

      strcpy( pc_command_mdadm, pc_template_mdadm );
      strcat( pc_command_mdadm, "/dev/" );
      strcat( pc_command_mdadm, ps_md_array_iter->name );
      strcat( pc_command_mdadm, " " );
      i = 0;
      while( NULL != ps_md_array_iter->devs[i] ) {
         strcat( pc_command_mdadm, ps_md_array_iter->devs[i] );
         strcat( pc_command_mdadm, " " );
      }

      i_command_mdadm_strlen += strlen( ps_md_array_iter->name );
      #endif

      /* Concat each device onto the command template. */
      /* TODO: Tweak this to allow more than two MD devices. */
      sprintf(
         ac_command_mdadm,
         "%s %s %s %s",
         pc_template_mdadm,
         ps_md_array_iter->name,
         ps_md_array_iter->strings[0],
         ps_md_array_iter->strings[1]
      );

      /* Close stdout/stderr if we're squelching errors. */
      #ifndef ERRORS
      dup2( 1, i_stdout_temp );
      dup2( 2, i_stderr_temp );
      i_null_fd = open( "/dev/null", O_WRONLY );
      dup2( i_null_fd, 1 );
      dup2( i_null_fd, 2 );
      #endif /* ERRORS */

      i_retval = system( ac_command_mdadm );

      /* Restore stdout/stderr. */
      #ifndef ERRORS
      dup2( i_stdout_temp, 1 );
      dup2( i_stderr_temp, 2 );
      close( i_null_fd );
      #endif /* ERRORS */

      /* free( pc_command_mdadm ); */
      
      printf( "%s\n", ac_command_mdadm );

      if( i_retval ) {
         #ifdef ERRORS
         PRINTF_ERROR(
            "There was a problem starting %s.\n", ps_md_array_iter->name 
         );
         #endif /* ERRORS */
         goto mm_cleanup;
      }

      ps_md_array_iter = ps_md_array_iter->next;
   }

mm_cleanup:

   /* Perform cleanup, destroy the information structure. */
   free( pc_template_mdadm );
   free( pc_md_arrays );
   config_free_string_holders( ps_md_arrays );

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
      * pc_path_mapper_s = NULL,
      * pc_root_mountpoint = NULL,
      * pc_fs_types_string = NULL,
      * pc_fs_iter;
   char** ppc_fs_types = NULL;

   /* Initialize strings, etc. */
   if( regcomp( &s_regex, ".*\\-root", 0 ) ) {
      #ifdef ERRORS
      perror( "Unable to compile root regex" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_REGEX_FAIL;
      goto mpr_cleanup;
   }

   pc_path_mapper = config_descramble_string(
      gac_sys_path_mapper,
      gai_sys_path_mapper
   );
   pc_path_mapper_s = config_descramble_string(
      gac_sys_path_mapper_s,
      gai_sys_path_mapper_s
   );
   pc_root_mountpoint = config_descramble_string(
      gac_sys_mpoint_root,
      gai_sys_mpoint_root
   );
   pc_fs_types_string = config_descramble_string(
      gac_sys_fs_types,
      gai_sys_fs_types
   );
   ppc_fs_types = config_split_string_array( pc_fs_types );

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
            sprintf( pc_root_dev, pc_path_mapper_s, p_dev_entry->d_name );
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
   pc_fs_iter = ppc_fs_types;
   while( NULL != pc_fs_iter ) {
   if( mount( pc_root_dev, pc_root_mountpoint, "ext3", MS_RDONLY, "" ) ) {
      /* Keep trying until we find an FS that works. */
      if( mount( pc_root_dev, pc_root_mountpoint, "ext2", MS_RDONLY, "" ) ) {
         #ifdef ERRORS
         /* printf( "%s %s\n", pc_root_dev, pc_root_mountpoint ); */
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
   free( pc_path_mapper_s );
   free( pc_root_mountpoint );
   free( pc_fs_types );
   config_free_string_array( ppc_fs_types );

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

