
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
   int i_retval = 0,
      i_hotplug_handle;
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

   /* Launch mdev to handle /dev FS. */
   #ifdef DEBUG
   printf( "Starting mdev...\n" );
   #endif /* DEBUG */

   i_hotplug_handle = open( "/proc/sys/kernel/hotplug", O_WRONLY );
   
   /* ERROR_PERROR(
      write( i_hotplug_handle, "/sbin/mdev", 10 ),
      i_retval,
      ERROR_RETVAL_MDEV_FAIL,
      ms_cleanup,
      "Unable to write mdev to /proc/sys/kernel/hotplug"
   ); */
   /* TODO: Should we squelch stdout/stderr for this? */
   ERROR_PRINTF(
      system( "/sbin/mdev -s" ),
      i_retval,
      ERROR_RETVAL_MDEV_FAIL,
      ms_cleanup,
      "Problem detected starting mdev.\n"
   );

   #ifdef DEBUG
   printf( "mdev started.\n" );
   #endif /* DEBUG */

ms_cleanup:

   close( i_hotplug_handle );

   return i_retval;
}

/* Purpose: Setup /dev/md devices if any exist.                               */
/* Return: 0 on success, 1 on failure.                                        */
int mount_mds( void ) {
   int i_command_mdadm_strlen = 0,
      i_retval = 0,
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
      console_hide();
      #endif /* ERRORS */

      i_retval = system( ac_command_mdadm );

      /* Restore stdout/stderr. */
      #ifndef ERRORS
      console_show();
      #endif /* ERRORS */

      /* free( pc_command_mdadm ); */
      
      /* printf( "%s\n", ac_command_mdadm ); */

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
   int i_retval = 0,
      i = 0;
   char* pc_root_dev = NULL,
      * pc_path_mapper = NULL,
      * pc_path_mapper_s = NULL,
      * pc_root_mountpoint = NULL,
      * pc_fs_types_string = NULL;
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
   ppc_fs_types = config_split_string_array( pc_fs_types_string );

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
         PRINTF_ERROR( "Unable to find root device.\n" );
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
   i = 0;
   while( NULL != ppc_fs_types[i] ) {
      #ifdef DEBUG
      printf( "Mounting %s as %s...\n", pc_root_dev, ppc_fs_types[i] );
      #endif /* DEBUG */
      i_retval = mount(
         pc_root_dev, pc_root_mountpoint, ppc_fs_types[i], MS_RDONLY, ""
      );

      if( !i_retval ) {
         /* This is kind of an odd duck, but here we skip to cleanup if we're *
          * successful, rather than if we fail. If we fail, the failure will  *
          * be set to the error value by default below.                       */

         goto mpr_cleanup;
      }

      /* Keep trying until we find an FS that works. */
      i++;
   }

   /* We weren't successful, so we don't skip. */
   #ifdef ERRORS
   /* printf( "%s %s\n", pc_root_dev, pc_root_mountpoint ); */
   perror( "Unable to mount root device" );
   #endif /* ERRORS */
   i_retval = ERROR_RETVAL_ROOT_FAIL;

mpr_cleanup:

   /* Cleanup. */
   regfree( &s_regex );
   free( pc_root_dev );
   free( pc_path_mapper );
   free( pc_path_mapper_s );
   free( pc_root_mountpoint );
   free( pc_fs_types_string );
   #if 0
   /* XXX: This is crashing if the the correct password is entered after >=1  *
    *      failed attempt.                                                    */
   config_free_string_array( ppc_fs_types );
   #endif

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

/* Portions of this code shamelessly stolen from busybox (but changed to      *
 * fit our preferred coding style.                                            */

static void mount_rmtree( char* pc_dir_path_in, dev_t i_root_dev_in ) {
   DIR* ps_dir;
   struct dirent* ps_entry;
   struct stat s_stat;
   char* pc_entry,
      * pc_last_char,
      * pc_entry_char_iter;

   /* Don't descend into other filesystems. */
   if( lstat( pc_dir_path_in, &s_stat ) || s_stat.st_dev != i_root_dev_in ) {
      goto mr_cleanup;
   }

   /* Recursively delete the contents of directories. */
   if( S_ISDIR( s_stat.st_mode ) ) {
      ps_dir = opendir( pc_dir_path_in );
      if( ps_dir ) {
         while( (ps_entry = readdir( ps_dir )) ) {
            pc_entry = ps_entry->d_name;

            /* Skip special entries. */
            if( (
               pc_entry[0] == '.' &&
               (pc_entry[1] == '\0' ||
                  (pc_entry[1] == '.' && pc_entry[2] == '\0'))
            ) ) {
               continue;
            }

            /* pc_entry =
               concat_path_file( pc_dir_path_in, pc_entry ); */
            pc_entry_char_iter = pc_entry;
            pc_last_char = last_char_is( pc_dir_path_in, '/' );
            while( '/' == *pc_entry_char_iter ) {
               pc_entry_char_iter++;
            }
            pc_entry = xasprintf(
               "%s%s%s",
               pc_dir_path_in,
               (NULL == pc_last_char ? "/" : ""),
               pc_entry
            );

            /* Recurse to delete contents. */
            mount_rmtree( pc_entry, i_root_dev_in );
            free( pc_entry );
         }
         closedir( ps_dir );

         /* Directory should now be empty; zap it. */
         rmdir( pc_dir_path_in );
      }
   } else {
      /* It wasn't a directory; zap it. */
      unlink( pc_dir_path_in );
   }

mr_cleanup:
   
   return;
}

/* Purpose: Switch from a tmpfs init root to a non-tmpfs full system root and *
 *          exec the proper init from there.                                  */
/* Return: Ideally, this will never return.                                   */
int mount_switch_root( char* pc_new_root_in ) {
   /* mount_rmtree( pc_new_root_in, NULL ); */

   struct stat s_stat;
   struct statfs s_statfs;
   dev_t i_root_dev;
   char* pc_command_switch_root_string,
      ** ppc_command_switch_root;
   int i_retval = 0;

   pc_command_switch_root_string = config_descramble_string(
      gac_command_switch_root,
      gai_command_switch_root
   );
   ppc_command_switch_root = config_split_string_array(
      pc_command_switch_root_string
   );

   /* Change to new root directory and verify it's a different FS. */
   ERROR_PRINTF(
      chdir( pc_new_root_in ),
      i_retval,
      ERROR_RETVAL_ROOT_FAIL,
      msr_cleanup,
      "Unable to chdir to new root.\n"
   );
   stat( "/", &s_stat );
   i_root_dev = s_stat.st_dev;
   stat( ".", &s_stat );
   if( s_stat.st_dev == i_root_dev || getpid() != 1 ) {
      #ifdef ERRORS
      PRINTF_ERROR( "Invalid call to switch_root.\n" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_ROOT_FAIL;
      goto msr_cleanup;
   }

   /* Additional sanity checks: we're about to rm -rf /, so be REALLY SURE we *
    * mean it.                                                                */
   if( stat( "/init", &s_stat ) != 0 || !S_ISREG( s_stat.st_mode ) ) {
      #ifdef ERRORS
      PRINTF_ERROR( "/init is not a regular file.\n" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_ROOT_FAIL;
      goto msr_cleanup;
   }

   statfs( "/", &s_statfs );
   if(
      RAMFS_MAGIC != (unsigned)s_statfs.f_type &&
      TMPFS_MAGIC != (unsigned)s_statfs.f_type
   ) {
      #ifdef ERRORS
      PRINTF_ERROR( "Root filesystem is not ramfs/tmpfs.\n" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_ROOT_FAIL;
      goto msr_cleanup;
   }

   /* Zap everything out of root dev. */
   mount_rmtree( "/", i_root_dev );

   /* Overmount / with newdir and chroot into it. */
   if( mount( ".", "/", NULL, MS_MOVE, NULL ) ) {
      #ifdef ERRORS
      PRINTF_ERROR( "Error moving root.\n" );
      #endif /* ERRORS */
      i_retval = ERROR_RETVAL_ROOT_FAIL;
      goto msr_cleanup;
   }
   ERROR_PRINTF(
      chroot( "." ),
      i_retval,
      ERROR_RETVAL_ROOT_FAIL,
      msr_cleanup,
      "Unable to chroot to new root.\n"
   );
   ERROR_PRINTF(
      chdir( "/" ),
      i_retval,
      ERROR_RETVAL_ROOT_FAIL,
      msr_cleanup,
      "Unable to refresh new root.\n"
   );

   #if 0
   /* If a new console specified, redirect stdin/stdout/stderr to it. */
   if( console ) {
      close(0);
      xopen(console, O_RDWR);
      xdup2(0, 1);
      xdup2(0, 2);
   }
   #endif

   execv( ppc_command_switch_root[0], ppc_command_switch_root );
 
msr_cleanup:

   /* Meaningless cleanup routines. */
   free( pc_command_switch_root_string );
   config_free_string_array( ppc_command_switch_root );

   return i_retval;
}

