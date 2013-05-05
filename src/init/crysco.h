
#ifndef CRYSCO_H
#define CRYSCO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>

#include "config.h"
#include "mount.h"
#include "error.h"
#include "console.h"

/* = Function Prototypes = */

int prompt_decrypt( void );

#endif /* CRYSCO_H */

