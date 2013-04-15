#!/usr/bin/env python

import argparse
import shutil
import tempfile
import os
import subprocess
import random
import re

def build_image():
   # FIXME: Copy the source to a working directory to make final tweaks.

   # TODO: Add kernel modules.
   pass

def compile_init( host_path, hostname ):
   try:
      temp_path = tempfile.mkdtemp()
      current_path = os.getcwd()

      # Copy the raw source.
      shutil.copytree(
         os.path.join( 'src', 'init' ),
         os.path.join( temp_path, 'init' )
      )

      # Copy the host-specific code.
      shutil.copy(
         os.path.join( host_path, '{}.h'.format( hostname ) ),
         os.path.join( temp_path, 'init', 'host.h' )
      )
      shutil.copy(
         os.path.join( host_path, '{}.c'.format( hostname ) ),
         os.path.join( temp_path, 'init', 'host.c' )
      )

      # Add the pseudo-random XOR key.
      xor_key = []
      for i in range( 0, 128 ):
         xor_key.append( str( random.randint( 0, 255 ) ) )

      with open( os.path.join( temp_path, 'init', 'host.c' ), 'r' ) as host_c:
         host_c_text = host_c.read()
      host_c_text = re.sub( r'::SKEY::', ', '.join( xor_key ), host_c_text )
      with open( os.path.join( temp_path, 'init', 'host.c' ), 'w' ) as host_c:
         host_c.write( host_c_text )

      # Perform the compile and copy the result back here.
      os.chdir( os.path.join( temp_path, 'init' ) )
      subprocess.call( ['make'] )
      os.chdir( current_path )
      try:
         os.mkdir( 'build' )
      except:
         pass
      shutil.copy(
         os.path.join( temp_path, 'init', 'init' ),
         os.path.join( '.', 'build', 'init' )
      )
   finally:
      try:
         shutil.rmtree( temp_path )
      except OSError, e:
         # Ignore no such directory errors.
         if 2 != e.errno:
            raise

def main():

   parser = argparse.ArgumentParser()
   parser.add_argument(
      '-c', '--compile-only', action='store_true', dest='compile_only',
      help='Only compile the C init; don\'t build the whole initrd.'
   )
   parser.add_argument(
      '-m', '--modules-path', action='store', dest='modules_path', type=str,
      help='Specify the path to kernel modules root to include.'
   )
   parser.add_argument(
      '-n', '--hostname', action='store', dest='hostname', type=str,
      help='Specify a hostname to compile for other than the current.'
   )
   parser.add_argument(
      '-p', '--host-path', action='store', dest='host_path',
      default='/etc/ifdy-initrd',
      help='Specify the path to host-specific configuration files.'
   )
   args = parser.parse_args()

   random.seed()

   compile_init( args.host_path, args.hostname )

   if not args.compile_only:
      build_image()

if __name__ == '__main__':
   main()

