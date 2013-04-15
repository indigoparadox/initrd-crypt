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

def copy_binary( binary_path, destination, static=None, root='/' ):

   ''' Attempt to copy a binary from a binlist. Return False on failure. '''

   my_logger = logging.getLogger( 'initrd.copy' )

   # Make sure the source file is where it should be.
   # TODO: Why do we look under /usr? Shouldn't the binlist specify that?
   binary_test = os.path.join( root, binary_path );
   if 's' == static and os.path.isfile( '{}.static'.format( binary_test ) ):
      binary_test = '{}.static'.format( binary_test )
   else:
      if not os.path.isfile( binary_test ):
         binary_test = os.path.join( root, 'usr', binary_path )
         if not os.path.isfile( binary_test ):
            my_logger.error(
               'Unable to locate {} for copy.'.format( binary_path )
            )
            return False

   my_logger.info( 'Copying {}...'.format( binary_test ) )

   # Make sure binary is static/dynamic as specified.
   binary_static = None
   proc_test = subprocess.Popen(
      ['ldd', binary_test], stdout=subprocess.PIPE, stderr=subprocess.PIPE
   )
   try:
      binary_static = proc_test.communicate()[0].index( 'not a dynamic' )
   except:
      pass

   if 'ld-linux.so.2' == os.path.basename( binary_test ):
      # This is a special case.
      binary_static = None

   if 's' == static and 1 != binary_static:
      my_logger.error(
         '{} should be static and is dynamic.'.format( binary_test )
      )
      return False
   elif 'l' == static and 1 == binary_static:
      my_logger.error(
         '{} should be dynamic and is static.'.format( binary_test )
      )
      return False

   # Perform the actual copy.
   shutil.copy(
      binary_test,
      os.path.join( destination, binary_path )
   )

   return True

def ld_version( root='/' ):

   ''' Return the current ld library version. '''

   ld_pattern = re.compile( r'ld-([0-9\.]*)\.so' )
   for entry in os.listdir( os.path.join( root, 'lib' ) ):
      ld_match = ld_pattern.match( entry )
      if None == ld_match:
         continue
      else:
         return ld_match.groups()[0]

def build_image( host_path, hostname, bincopy_root='/' ):
   try:
      my_logger = logging.getLogger( 'initrd.build' )

      temp_path = tempfile.mkdtemp()
      current_path = os.getcwd()

      # Copy the source to a working directory to make final tweaks.
      shutil.copytree(
         # FIXME: Specify image template path.
         'image',
         os.path.join( temp_path, 'initrd' ),
         symlinks=True
      )

      # TODO: Add kernel modules.

      os.chdir( os.path.join( temp_path, 'initrd' ) )

      # TODO: Perform the integration from the host-specific binary list.
      ld_ver = ld_version( root=bincopy_root )
      with open(
         os.path.join( host_path, '{}.binlist'.format( hostname ) ), 'r'
      ) as binlist_file:
         binline_pattern = re.compile( r'(\S*)\s*([sl]?)' )
         for line in binlist_file:
            binline_match = binline_pattern.match( line )
            line_path = binline_match.groups()[0].format( ld_ver )
            if not copy_binary(
               line_path,
               os.path.join( temp_path, 'initrd' ),
               static=binline_match.groups()[1],
               root=bincopy_root
            ):
               my_logger.error( 'Error copying file. Aborting.' )
               return

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
      my_logger.info( 'Compressing image...' )
      proc_find = \
         subprocess.Popen( ['find', '.', '-print0'], stdout=subprocess.PIPE )
      proc_cpio = subprocess.Popen(
         ['cpio', '--null', '-ov', '--format=newc'],
         stdin=proc_find.stdout,
         stdout=subprocess.PIPE,
         stderr=subprocess.PIPE
      )

      # Open up the target archive file and dump the compressed data into it.
      # FIXME: Specify the destination path.
      image_path = os.path.join( current_path, 'build', 'initrd.gz' )
      with gzip.open( image_path, 'wb', compresslevel=9 ) as output:
         for chunk in iter( lambda: proc_cpio.stdout.read( 8192 ), '' ):
            output.write( chunk )

      my_logger.info( 'Image written to {}.'.format( image_path ) )

      os.chdir( current_path )

   finally:
      try:
         shutil.rmtree( temp_path )
      except OSError, e:
         # Ignore no such directory errors.
         if 2 != e.errno:
            raise

def compile_init( host_path, hostname, release=False ):
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

      scramble_strings( os.path.join( temp_path, 'init' ) )

      # Perform the compile and copy the result back here.
      os.chdir( os.path.join( temp_path, 'init' ) )
      command = ['make']
      if release:
         command += ['release']
      subprocess.check_call( command )
      os.chdir( current_path )
      try:
         os.mkdir( 'build' )
      except:
         pass
      shutil.copy(
         os.path.join( temp_path, 'init', 'init' ),
         # FIXME: Specify the destination path.
         os.path.join( '.', 'build', 'init' )
      )
      shutil.copy(
         os.path.join( temp_path, 'init', 'host.c' ),
         # FIXME: Specify the destination path.
         os.path.join( '.', 'build', 'host.c' )
      )
   finally:
      try:
         shutil.rmtree( temp_path )
      except OSError, e:
         # Ignore no such directory errors.
         if 2 != e.errno:
            raise

def scramble_strings( init_path ):

   host_c_path = os.path.join( init_path, 'host.c' )

   # Generate the pseudo-random XOR key.
   xor_key = []
   for i in range( 0, 128 ):
      xor_key.append( str( random.randint( 1, 255 ) ) )

   with open( host_c_path, 'r' ) as host_c:
      host_c_text = host_c.read()

   # Add the key and scramble appropriate strings.
   host_c_text = re.sub( r'::SKEY::', ', '.join( xor_key ), host_c_text )
   for string in re.findall( r'bfromcstr\( "(.*)" \)', host_c_text ):
      host_c_text = re.sub(
         r'bfromcstr\( "{}" \)'.format( string ),
         r'{{ {} }}'.format( 
            ', '.join( _scramble_string_iter( string, xor_key ) )
         ),
         host_c_text
      )

   with open( host_c_path, 'w' ) as host_c:
      host_c.write( host_c_text )

   #input( "Enter to continue..." )

def _scramble_string_iter( string, key ):
   string_out = []
   for string_char, key_char in zip( string, key ):
      string_out.append( str( ord( string_char ) ^ int( key_char ) ) )
   # Append a null terminator.
   string_out.append( '0' )
   return string_out

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
   compile_init( args.host_path, args.hostname, release=not args.compile_only )

   if not args.compile_only:
      my_logger.info( 'Building image...' )
      build_image( args.host_path, args.hostname )

if __name__ == '__main__':
   main()

