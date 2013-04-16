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

def build_image( args, temp_path ):
   my_logger = logging.getLogger( 'initrd.build' )

   current_path = os.getcwd()
   initrd_path = os.path.join( temp_path, 'initrd' )

   # Copy the source to a working directory to make final tweaks.
   shutil.copytree(
      args.image_path,
      initrd_path,
      symlinks=True
   )
   shutil.copy(
      os.path.join( args.output_path, 'init' ),
      initrd_path
   )

   # FIXME: Copy compiled init to the working directory.

   # TODO: Add kernel modules.

   os.chdir( initrd_path )

   # TODO: Perform the integration from the host-specific binary list.
   ld_ver = ld_version( root=args.root_path )
   with open(
      os.path.join( args.host_path, '{}.binlist'.format( args.hostname ) ),
      'r'
   ) as binlist_file:
      binline_pattern = re.compile( r'(\S*)\s*([sl]?)' )
      for line in binlist_file:
         binline_match = binline_pattern.match( line )
         line_path = binline_match.groups()[0].format( ld_ver )
         if not copy_binary(
            line_path,
            initrd_path,
            static=binline_match.groups()[1],
            root=args.root_path
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
   os.chdir( current_path )
   image_path = os.path.join( args.output_path, 'initrd.gz' )
   with gzip.open( image_path, 'wb', compresslevel=9 ) as output:
      for chunk in iter( lambda: proc_cpio.stdout.read( 8192 ), '' ):
         output.write( chunk )

   my_logger.info( 'Image written to {}.'.format( image_path ) )

#def compile_init( host_path, hostname, release=False, errors=False, net=False ):
def compile_init( args, temp_path ):
   my_logger = logging.getLogger( 'initrd.compile' )
   current_path = os.getcwd()
   init_build_path = os.path.join( temp_path, 'init' )

   # Copy the raw source.
   shutil.copytree(
      # FIXME: Specify source path.
      os.path.join( 'src', 'init' ),
      init_build_path
   )

   # Copy the host-specific code.
   shutil.copy(
      os.path.join( args.host_path, '{}.c'.format( args.hostname ) ),
      os.path.join( init_build_path, 'host.c' )
   )

   my_xor_key = xor_key()
   scramble_strings(
      os.path.join( temp_path, 'init' ),
      os.path.join( init_build_path, 'host.c' ),
      my_xor_key
   )
   scramble_strings(
      os.path.join( temp_path, 'init' ),
      os.path.join( init_build_path, 'genstrings.c' ),
      my_xor_key
   )

   shutil.copy(
      os.path.join( temp_path, 'init', 'host.c' ),
      os.path.join( args.output_path, 'host.c' )
   )
   shutil.copy(
      os.path.join( temp_path, 'init', 'genstrings.c' ),
      os.path.join( args.output_path, 'genstrings.c' )
   )

   # Perform the compile and copy the result back here.
   os.chdir( init_build_path )
   command = ['make']

   # Add options to build command.
   cflags = ['CFLAGS=-Wall -O3']
   if not args.compile_only:
      command += ['release']
   else:
      cflags += ['-g']

   if args.errors:
      cflags += ['-DERRORS']

   if args.internet:
      cflags += ['-DNET']

   if args.console:
      cflags += ['-DCONSOLE']

   command += [' '.join( cflags )]

   try:
      subprocess.check_call( command )
   except:
      my_logger.error( "Make process failed." )
      return
   os.chdir( current_path )
   try:
      os.mkdir( 'build' )
   except:
      pass
   shutil.copy(
      os.path.join( init_build_path, 'init' ),
      os.path.join( args.output_path, 'init' )
   )

def scramble_strings( init_path, source_path, xor_key ):

   with open( source_path, 'r' ) as source_file:
      source_text = source_file.read()

   # Add the key and scramble appropriate strings.
   source_text = re.sub( r'::SKEY::', ', '.join( xor_key ), source_text )
   for string in re.findall( r'bfromcstr\( "(.*)" \)', source_text ):
      source_text = re.sub(
         r'bfromcstr\( "{}" \)'.format( string ),
         r'{{ {} }}'.format( 
            ', '.join( _scramble_string_iter( string, xor_key ) )
         ),
         source_text
      )

   with open( source_path, 'w' ) as source_file:
      source_file.write( source_text )

   #input( "Enter to continue..." )

def _scramble_string_iter( string, key ):
   string_out = []
   for string_char, key_char in zip( string, key ):
      string_out.append( str( ord( string_char ) ^ int( key_char ) ) )
   # Append a null terminator.
   string_out.append( '0' )
   return string_out

def xor_key():
   # Generate the pseudo-random XOR key.
   xor_key = []
   for i in range( 0, 128 ):
      xor_key.append( str( random.randint( 1, 255 ) ) )
   return xor_key

def main():

   parser = argparse.ArgumentParser()
   parser.add_argument(
      '-c', '--compile-only', action='store_true', dest='compile_only',
      help='Only compile the C init; don\'t build the whole initrd.'
   )
   parser.add_argument(
      '-e', '--errors', action='store_true', dest='errors',
      help='Build an init that will show meaningful errors.'
   )
   parser.add_argument(
      '-i', '--internet', action='store_true', dest='internet',
      help='Build an init with Internet support.'
   )
   parser.add_argument(
      '-t', '--console', action='store_true', dest='console',
      help='Build an init with a console option.'
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
      '-p', '--host-path', action='store', dest='host_path', type=str,
      default='/etc/ifdy-initrd',
      help='Specify the path to host-specific configuration files.'
   )
   parser.add_argument(
      '-g', '--image-path', action='store', dest='image_path', type=str,
      default='/usr/share/ifdy-initrd/image',
      help='Specify the path to initrd image skeleton.'
   )
   parser.add_argument(
      '-r', '--root-path', action='store', dest='root_path', type=str,
      default='/',
      help='Specify the path to the root from which to copy binaries.'
   )
   parser.add_argument(
      '-o', '--output-path', action='store', dest='output_path', type=str,
      default='.',
      help='Specify the path in which to store the built initrd.'
   )
   args = parser.parse_args()

   logging.basicConfig( level=logging.INFO )
   my_logger = logging.getLogger( 'initrd' )

   random.seed()

   try:
      temp_path = tempfile.mkdtemp()
      my_logger.info( 'Compiling init...' )
      compile_init( args, temp_path )

      if not args.compile_only:
         my_logger.info( 'Building image...' )
         build_image( args, temp_path )
   finally:
      try:
         shutil.rmtree( temp_path )
      except OSError, e:
         # Ignore no such directory errors.
         if 2 != e.errno:
            raise

if __name__ == '__main__':
   main()

