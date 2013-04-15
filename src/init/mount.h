
#ifndef MOUNT_H
#define MOUNT_H

#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <regex.h>

#include "host.h"

int mount_sys( BOOL b_umount_in );
int mount_mds( void );
int mount_probe_root( void );
int mount_probe_usr( void );

#endif /* MOUNT_H */
