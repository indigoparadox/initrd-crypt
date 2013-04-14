
#ifndef DATATYPES_H
#define DATATYPES_H

#include "bstrlib/bstrlib.h"

#include <stdlib.h>

typedef struct {
   bstring name;
   struct bstrList* devs;
} MD_ARRAY;

#endif /* DATATYPES_H */

