
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

/* = Function Prototypes = */

void console_hide( void );
void console_show( void );
void console_echo_off( void );
void console_echo_on( void );

#endif /* CONSOLE_H */

