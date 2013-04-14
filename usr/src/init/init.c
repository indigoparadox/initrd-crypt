
#include <stdio.h>
#include <dirent.h>

#include "bstrlib/bstrlib.h"

#ifndef HOSTNAME_H
#error "No hostname specified!"
#endif /* HOSTNAME */

#include "crysco.h"

int probe_block( void ) {
   DIR* p_dev_dir;
   struct dirent* p_dev_entry;

   p_dev_dir = opendir( "/dev" );
   if( NULL != p_dev_dir ) {
      while( p_dev_entry = readdir( p_dev_dir ) ) {
         /* Is this a block device? */
         /* FIXME */
      }
   } else {
      perror( "Unable to open /dev. Aborting." );
      return -1;
   }

   perror( "Unable to locate suitable block device. Aborting." );
   return -1;
}

int probe_usr( void ) {
   /* TODO: Open fstab and see if a line exists for /usr. Mount if applicable.
    */


}

int main( void ) {

   /* Mount system directories. */

   /* TODO: Start the splash screen (deprecated). */

   /* Load any directed kernel modules. */

   return 0;
}

