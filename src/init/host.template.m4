divert(-1)
dnl changequote(<:>)
dnl changequote(`,')
dnl #include "host.h"
dnl esyscmd(`scripts/xor.sh -f maketemp(`foo') $1')
dnl esyscmd(`rm TEMPFILE()')')
changecom(@@)
define(`TEMPFILE',maketemp(`/tmp/irds'))
esyscmd(./scripts/xor.sh -k -f TEMPFILE())
define(`CONFIG_SCR',`#define $1 esyscmd(./scripts/xor.sh -f TEMPFILE() "$2")')
define(`CONFIG_RAW',`#define $1 $2')
define(`CONFIG_END',`
/* = Function Prototypes = */

char* config_descramble_string( const char* );
char** config_split_string_array( const char*, char* );

MD_ARRAY* config_load_md_arrays( void );
void config_free_md_arrays( MD_ARRAY* );

#endif /* SCRAMBLES_H */')
divert(0)
#ifndef SCRAMBLES_H
#define SCRAMBLES_H

#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define CONFIG_STRING_MAX_LEN 255
#define CONFIG_STRING_ARRAY_MAX_LEN 20

/* = Structures and Types = */

typedef struct MD_ARRAY {
   char* name;
   char** devs;
   struct MD_ARRAY* next;
} MD_ARRAY;

typedef struct LUKS_VOL {
   char* name;
   char* dev;
   char* fs;
   struct LUKS_VOL* next;
} LUKS_VOL;

/* = Macros = */

#define CONFIG_SCRAMBLED_STRING( scrambled_string ) \
   extern const char* scrambled_string;

#define CONFIG_FREE_STRING_ARRAY( string_array ) \
   while( NULL != string_array[i] ) { \
      free( string_array[i] ); \
   } \
   free( string_array );

/* = Key Configuration = */

/* This must occur before ANY strings can be scrambled! */

#define CONFIG_SKEY esyscmd(./scripts/xor.sh -f TEMPFILE() -k)

/* = Generic Configuration = */

/* This only supports two devices per array right now, but maybe we'll        *
 * support more later on.                                                     */
CONFIG_SCR(CONFIG_REGEX_MD_ARRAYS,`[a-zA-Z0-9]*<[a-zA-Z0-9]*|[a-zA-Z0-9]*>')
CONFIG_SCR(CONFIG_REGEX_STRING_ARRAY,`\\([^|]*\\)')

CONFIG_SCR(CONFIG_SYS_FS_MOUNT,`/sys|/proc|/dev|/dev/pts')
CONFIG_SCR(CONFIG_SYS_MPOINT_MOUNT,`/sys|/proc|/dev|/dev/pts')
CONFIG_SCR(CONFIG_SYS_MTYPE_MOUNT,`sysfs|proc|devtmpfs|devpts')
CONFIG_SCR(CONFIG_SYS_FS_UMOUNT,`/dev/pts|/dev|/proc|/sys')
CONFIG_SCR(CONFIG_SYS_MPOINT_ROOT,`/mnt/root')
CONFIG_SCR(CONFIG_SYS_PATH_MAPPER,`/dev/mapper')
CONFIG_SCR(CONFIG_COMMAND_MDADM,`mdadm --assemble')

/* = Host-Specific Configuration = */

