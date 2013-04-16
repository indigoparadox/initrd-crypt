
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
   prompt_decrypt();

   /* FIXME: Return 0 on successful decrypt. */
   return 0;
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
   b_decrypt_success = !action_crypt();

main_cleanup:
   if( 1 == getpid() ) {
      /* Prepare the system to load the "real" init. */
      if( b_decrypt_success ) {
         mount_probe_usr();
      }
      mount_sys( FALSE );
   }

   return 0;
}

