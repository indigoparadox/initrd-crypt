#!/usr/bin/env python

import argparse
import shutil
import tempfile
import os
import subprocess
import random
import re
import logging
import gzip

def fix_perms( path, fmod, recursive=False, ftype=None, fname=None ):

   ''' Fix permissions for the given path to the given mod. Returns True on
   success. '''
   
   if recursive:
      command = ['find', '.']
      if None != ftype:
         command += ['-type', ftype]
      if None != fname:
         command += ['-name', fname]
      command += ['-exec', 'chmod', fmod, '{}', ';']
   else:
      command = ['chmod', fmod, path]

   try:
      subprocess.check_call( command )
   except:
      raise

   return True

def build_image():
   try:
      my_logger = logging.getLogger( 'initrd.build' )

      temp_path = tempfile.mkdtemp()
      current_path = os.getcwd()

      # FIXME: Copy the source to a working directory to make final tweaks.
      shutil.copytree(
         # FIXME: Specify image template path.
         'image',
         os.path.join( temp_path, 'initrd' ),
         symlinks=True
      )

      # TODO: Add kernel modules.

      os.chdir( os.path.join( temp_path, 'initrd' ) )

      # TODO: Perform the integration from the host-specific binary list.

      # Fix internal permissions and ownership.
      my_logger.info( 'Fixing permissions...' )
      fix_perms( '.', '755', recursive=True, ftype='d' )
      fix_perms( '.', '644', recursive=True, ftype='f' )
      fix_perms( './bin', '755', recursive=True, ftype='f' )
      fix_perms( './sbin', '755', recursive=True, ftype='f' )
      fix_perms( './lib', '755', recursive=True, fname='*.so' )
      fix_perms( './etc/profile', '755' )
      fix_perms( './init', '755' )
      #fix_perms( './sbin/dropbear', '4755' )
      #fix_perms( './bin/busybox', '4755' )
      #fix_perms( './bin/simple.script', '4755' )
      #my_logger.info( 'Fixing ownership...' )
      #subprocess.check_call( ['sudo', 'chown', '-R', 'root:root', '.'] )

      # TODO: Randomize or allow specification of a file modification date.

      # Perform the actual build of the image.
      proc_find = \
         subprocess.Popen( ['find', '.', '-print0'], stdout=subprocess.PIPE )
      proc_cpio = subprocess.Popen(
         ['cpio', '--null', '-ov', '--format=newc'],
         stdin=proc_find.stdout,
         stdout=subprocess.PIPE
      )

      # Open up the target archive file and dump the compressed data into it.
      image_path = os.path.join( current_path, 'initrd.gz' )
      with gzip.open( image_path, 'wb', compresslevel=9 ) as output:
         for chunk in iter( lambda: proc_cpio.stdout.read( 8192 ), '' ):
            output.write( chunk )

      os.chdir( current_path )

   finally:
      try:
         shutil.rmtree( temp_path )
      except OSError, e:
         # Ignore no such directory errors.
         if 2 != e.errno:
            raise

def compile_init( host_path, hostname ):
   try:
      temp_path = tempfile.mkdtemp()
      current_path = os.getcwd()

      # Copy the raw source.
      shutil.copytree(
         # FIXME: Specify source path.
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

   logging.basicConfig( level=logging.INFO )
   my_logger = logging.getLogger( 'initrd' )

   random.seed()

   my_logger.info( 'Compiling init...' )
   compile_init( args.host_path, args.hostname )

   if not args.compile_only:
      my_logger.info( 'Building image...' )
      build_image()

if __name__ == '__main__':
   main()

