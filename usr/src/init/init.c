
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mount.h>

#include "bstrlib/bstrlib.h"

#ifndef HOSTNAME_H
#error "No hostname specified!"
#endif /* HOSTNAME */

#include "crysco.h"

/* Purpose: Mount /usr before boot for compatbility with newer systems.       */
int probe_usr( void ) {
   /* TODO: Open fstab and see if a line exists for /usr. Mount if applicable.
    */
   return -1;
}

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
      if(
         mount( NULL, "/dev", "devtmpfs", 0, "" ) ||
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
   MD_ARRAY* md_arrays;
   
   i_md_count = get_md_arrays( &md_arrays );

   /* Iterate through the host-specific data structure and create md arrays.  */
   for( i_md_iter = 0 ; i_md_count > i_md_iter ; i_md_iter++ ) {
      /* printf( "%s\n", bdata( md_arrays[i_md_iter].name ) ); */
      for(
         i_dev_iter = 0 ;
         md_arrays[i_md_iter].devs->qty > i_dev_iter ;
         i_dev_iter++
      ) {
         /* FIXME: Actually perform array creation. */
         /* printf(
            "%s\n", bdata( md_arrays[i_md_iter].devs->entry[i_dev_iter] )
         ); */
      }
   }

   /* Perform cleanup, destroy the information structure. */
   for( i_md_iter = 0 ; i_md_count > i_md_iter ; i_md_iter++ ) {
      bstrListDestroy( md_arrays[i_md_iter].devs );
      bdestroy( md_arrays[i_md_iter].name );
   }
   free( md_arrays );

   /* FIXME: Abort if there's a problem creating arrays. */
   return 0;
}

/* Purpose: Wait until the main devices are decrypted and start the system.   */
void action_crypt( void ) {

   /* FIXME: Try to start network listener. */

   /* Get the user password. */
   prompt_decrypt();

   /* Try to premount /usr, because of the new udev. */
   probe_usr();
}

int main( int argc, char* argv[] ) {
   int i;
   BOOL b_decrypt_success = FALSE;

   if( 1 == getpid() ) {
      /* We're being called as init, so set the system up. */
      mount_sys( FALSE );
      mount_mds();

      /* TODO: Load any directed kernel modules. */

      /* TODO: Start the splash screen (deprecated). */
   }

   /* See if we're being called as a prompt only. */
   for( i = 1 ; i < argc ; i++ ) {
      if( !strncmp( "-p", argv[i], 2 ) ) {
         /* Just prompt to decrypt and exit (signaling main process to clean  *
          * up if decrypt is successful!)                                     */
         if( !prompt_decrypt() ) {
         }
         goto main_cleanup;
      }
   }

   /* Act based on the system imperative. */
   action_crypt();

main_cleanup:
   if( 1 == getpid() ) {
      /* Prepare the system to load the "real" init. */
      if( b_decrypt_success ) {
         probe_usr();
      }
      mount_sys( FALSE );
   }

   return 0;
}

