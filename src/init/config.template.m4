divert(-1)
dnl = Configuration Settings =
dnl
divert(0)
/* = Generic Configuration = */

CONFIG_SCR(sys_fs_mount,`/sys|/proc|/dev|/dev/pts')
CONFIG_SCR(sys_mpoint_mount,`/sys|/proc|/dev|/dev/pts')
CONFIG_SCR(sys_mtype_mount,`sysfs|proc|devtmpfs|devpts')
CONFIG_SCR(sys_fs_umount,`/dev/pts|/dev|/proc|/sys')
CONFIG_SCR(sys_mpoint_root,`/mnt/root')
CONFIG_SCR(sys_path_mapper,`/dev/mapper')
CONFIG_SCR(command_mdadm,`mdadm --assemble')

/* = Host-Specific Configuration = */
