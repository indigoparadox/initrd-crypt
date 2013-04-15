
#ifndef DATATYPES_H
#define DATATYPES_H

#include <stdlib.h>
#include <string.h>

typedef struct {
   char* name;
   int devs_count;
   char** devs;
} MD_ARRAY;

typedef struct {
   char* name;
   char* dev;
} LVOL;

#ifndef BOOL
typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif /* TRUE */
#endif /* BOOL */

char read_char_static( char, char );
void decode_copy_string( char*, char*, char* );

#endif /* DATATYPES_H */

