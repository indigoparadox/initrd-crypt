
#include <stdio.h>
#include <unistd.h>

#include "host.h"

#ifndef HOSTNAME_H
#error "No hostname specified!"
#endif /* HOSTNAME */

#include "crysco.h"
#include "mount.h"

/* Purpose: Wait until the main devices are decrypted and start the system.   */
int action_crypt( void ) {

   #ifdef NET
   /* TODO: Try to start network listener. */
   #endif /* NET */

   /* Get the user password. */
   return prompt_decrypt();
}

int main( int argc, char* argv[] ) {
   int i,
      i_retval = 0;

   if( 1 == getpid() ) {
      /* We're being called as init, so set the system up. */
      i_retval = mount_sys( FALSE );
      if( i_retval ) {
         goto main_cleanup;
      }
      i_retval = mount_mds();
      if( i_retval ) {
         goto main_cleanup;
      }

      /* TODO: Load any directed kernel modules. */

      /* TODO: Start the splash screen (deprecated). */
   }

   /* See if we're being called as a prompt only. */
   for( i = 1 ; i < argc ; i++ ) {
      if( !strncmp( "-p", argv[i], 2 ) ) {
         /* Just prompt to decrypt and exit (signaling main process to clean  *
          * up if decrypt is successful!)                                     */
         if( prompt_decrypt() ) {
         }
         goto main_cleanup;
      }
   }

   /* Act based on the system imperative. */
   i_retval = action_crypt();

main_cleanup:
   if( 1 == getpid() ) {
      /* Prepare the system to load the "real" init. */
      if( !i_retval ) {
         i_retval = mount_probe_usr();
      }
      if( !i_retval ) {
         i_retval = mount_sys( FALSE );
      }

      /* FIXME: Execute switchroot. */
      if( !i_retval ) {
      }
   }

   return i_retval;
}

