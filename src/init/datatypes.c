
#include "datatypes.h"

char read_static( char x_in, char y_in ) {
   return ~((x_in & y_in) | (~x_in & ~y_in));
}

