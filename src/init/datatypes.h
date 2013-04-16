
#ifndef DATATYPES_H
#define DATATYPES_H

typedef struct {
   char* name;
   int devs_count;
   char** devs;
} MD_ARRAY;

typedef struct {
   char* name;
   char* dev;
   char* fs;
} LVOL;

#ifndef BOOL
typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif /* TRUE */
#endif /* BOOL */

#endif /* DATATYPES_H */

