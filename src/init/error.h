
#ifndef ERROR_H
#define ERROR_H

#define ERROR_RETVAL_ROOT_FAIL 1
#define ERROR_RETVAL_MAPPER_FAIL 2
#define ERROR_RETVAL_SYSFS_FAIL 4
#define ERROR_RETVAL_LVM_FAIL 8
#define ERROR_RETVAL_REGEX_FAIL 16
#define ERROR_RETVAL_ACTION_FAIL 32
#define ERROR_RETVAL_DECRYPT_FAIL 64 /* Doesn't count root probe failure. */

/* = Macros = */
#define PRINTF_ERROR( ... ) fprintf( stderr, __VA_ARGS__ );

#endif /* ERROR_H */

