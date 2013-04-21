
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* = Function Prototypes = */

void console_hide( void );
void console_show( void );

#endif /* CONSOLE_H */

