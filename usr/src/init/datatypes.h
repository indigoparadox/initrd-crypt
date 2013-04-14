
#ifndef DATATYPES_H
#define DATATYPES_H

#include "bstrlib/bstrlib.h"

#include <stdlib.h>

typedef struct {
   bstring name;
   struct bstrList* devs;
} MD_ARRAY;

#ifndef BOOL
typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif /* TRUE */
#endif /* BOOL */

#endif /* DATATYPES_H */

