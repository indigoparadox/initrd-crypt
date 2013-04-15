#!/usr/bin/env python

import argparse;

def main():

   parser = argparse.ArgumentParser()
   parser.add_argument(
      '-m', '--modules-path', action='store', dest='modules_path', type=str,
      help='Specify the path to kernel modules root to include.'
   )
   #parser.add_argument(
   #   '-h', '--hostname', action='store', dest='hostname', type=str,
   #   help='Specify a hostname to compile for other than the current.'
   #)
   args = parser.parse_args();

if __name__ == '__main__':
   main()

