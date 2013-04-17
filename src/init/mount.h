
#ifndef MOUNT_H
#define MOUNT_H

#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <regex.h>
#include <unistd.h>
#include <fcntl.h>

#include "genstrings.h"
#include "host.h"
#include "error.h"

/* = Function Prototypes = */

int umount_sys( void );
int mount_sys( void );
int mount_mds( void );
int mount_probe_root( void );
int mount_probe_usr( void );

#endif /* MOUNT_H */

