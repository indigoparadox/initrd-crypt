
#ifndef MOUNT_H
#define MOUNT_H

#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <regex.h>
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/vfs.h>
#include <sys/mount.h>

/* Make up for header deficiencies. */
#ifndef RAMFS_MAGIC
# define RAMFS_MAGIC ((unsigned)0x858458f6)
#endif
#ifndef TMPFS_MAGIC
# define TMPFS_MAGIC ((unsigned)0x01021994)
#endif
#ifndef MS_MOVE
# define MS_MOVE     8192
#endif

#include "config.h"
#include "error.h"
#include "console.h"
#include "util.h"

/* = Function Prototypes = */

int umount_sys( void );
int mount_sys( void );
int mount_mds( void );
int mount_probe_root( void );
int mount_probe_usr( void );
int mount_switch_root( char* );

#endif /* MOUNT_H */

