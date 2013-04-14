
#include <stdio.h>
#include <dirent.h>

#include "bstrlib/bstrlib.h"

#ifndef HOSTNAME_H
#error "No hostname specified!"
#endif /* HOSTNAME */

#include "crysco.h"

int probe_usr( void ) {
   /* TODO: Open fstab and see if a line exists for /usr. Mount if applicable.
    */
   return -1;
}

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

void action_crypt( void ) {

}

int main( int argc, char* argv[] ) {
   int i;

   mount_mds();

   for( i = 1 ; i < argc ; i++ ) {
      if( !strncmp( "-p", argv[i], 2 ) ) {
         /* Just prompt to decrypt and exit (signalling main process to clean *
          * up if decrypt is successful!)                                     */
         if( !prompt_decrypt() ) {
         }
         goto main_cleanup;
      }
   }

   action_crypt();

   /* Mount system directories. */

   /* TODO: Start the splash screen (deprecated). */

   /* Load any directed kernel modules. */

main_cleanup:

   return 0;
}

