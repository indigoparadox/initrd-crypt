
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

#define CONFIG_FREE_STRING_ARRAY( string_array ) \
   while( NULL != string_array[i] ) { \
      free( string_array[i] ); \
   } \
   free( string_array );

/* = Function Prototypes = */

char* config_descramble_string( const char*, const int );
char** config_split_string_array( const char*, char* );

MD_ARRAY* config_load_md_arrays( void );
void config_free_md_arrays( MD_ARRAY* );

#endif /* SCRAMBLES_H */

