
#ifndef ERROR_H
#define ERROR_H

#define ERROR_RETVAL_ROOT_FAIL 1
#define ERROR_RETVAL_MAPPER_FAIL 2
#define ERROR_RETVAL_SYSFS_FAIL 4
#define ERROR_RETVAL_LVM_FAIL 8
#define ERROR_RETVAL_REGEX_FAIL 16
#define ERROR_RETVAL_ACTION_FAIL 32
#define ERROR_RETVAL_DECRYPT_FAIL 64 /* Doesn't count root probe failure. */
#define ERROR_RETVAL_CONSOLE_DONE 128 /* Not technically an error. */
#define ERROR_RETVAL_NET_FAIL 256
#define ERROR_RETVAL_SERIAL_FAIL 512
#define ERROR_RETVAL_MDEV_FAIL 1024
#define ERROR_RETVAL_SSH_FAIL 2048
#define ERROR_RETVAL_VLAN_FAIL 4096
#define ERROR_RETVAL_SWITCH_FAIL 8192
#define ERROR_RETVAL_TOR_FAIL 16384
#define ERROR_RETVAL_EXEC_FAIL 32768

/* = Macros = */

#ifdef DEBUG
#define PRINTF_DEBUG( ... ) printf( __VA_ARGS__ );
#else
#define PRINTF_DEBUG( ... )
#endif /* DEBUG */

/* TODO: Get rid of this outdated macro. */
#define PRINTF_ERROR( ... ) fprintf( stderr, __VA_ARGS__ );

#ifdef ERRORS 

#define ERROR_PRINTF( test, retval, errno, golabel, ... ) \
   if( test ) { \
      fprintf( stderr, __VA_ARGS__ ); \
      retval |= errno; \
      goto golabel; \
   }

#define ERROR_PRINTF_SYSTEM( command, retval, errno, golabel, ... ) \
   if( system( command ) ) { \
      fprintf( stderr, __VA_ARGS__ ); \
      retval |= errno; \
      goto golabel; \
   }

#define ERROR_PERROR( test, retval, errno, golabel, errmsg ) \
   if( test ) { \
      perror( errmsg ); \
      retval |= errno; \
      goto golabel; \
   }

#define ERROR_PERROR_NOBREAK( test, retval, errno, errmsg ) \
   if( test ) { \
      perror( errmsg ); \
      retval |= errno; \
   }

#else

#define ERROR_PRINTF( test, retval, errno, golabel, ... ) \
   if( test ) { \
      if( errno ) { \
         retval |= errno; \
      } \
      goto golabel; \
   }

#define ERROR_PRINTF_SYSTEM( command, retval, errno, golabel, ... ) \
   console_hide(); \
   if( system( command ) ) { \
      if( errno ) { \
         retval |= errno; \
      } \
      goto golabel; \
   } \
   console_show();

#define ERROR_PERROR( test, retval, errno, golabel, errmsg ) \
   if( test ) { \
      retval |= errno; \
      goto golabel; \
   }

#define ERROR_PERROR_NOBREAK( test, errmsg ) \
   if( test ) { \
      retval |= errno; \
   }

#endif /* ERRORS */
   

#endif /* ERROR_H */

