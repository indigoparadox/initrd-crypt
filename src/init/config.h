
#ifndef SCRAMBLES_H
#define SCRAMBLES_H

#include <stdlib.h>
#include <string.h>
#include <regex.h>

/* = Structures and Types = */

struct string_holder {
   char* name;
   char** strings;
   struct string_holder* next;
};

/* = Function Prototypes = */

char* config_descramble_string( const char*, const int );
char** config_split_string_array( const char* );
void config_free_string_array( char** );
struct string_holder* config_split_string_holders( const char* );
void config_free_string_holders( struct string_holder* );

#endif /* SCRAMBLES_H */

