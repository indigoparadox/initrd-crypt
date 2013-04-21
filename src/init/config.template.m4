divert(-1)
dnl = Configuration Settings =
dnl
divert(0)
/* = Generic Configuration = */

CONFIG_SCR(
   `sys_fs_mount',
   `/sys<none|sysfs>/proc<none|proc>/dev<none|devtmpfs>/dev/pts<none|devpts>'
)
CONFIG_SCR(sys_fs_types,`ext3|ext2|xfs|reiserfs')
CONFIG_SCR(sys_mpoint_root,`/mnt/root')
CONFIG_SCR(sys_path_mapper,`/dev/mapper')
CONFIG_SCR(sys_path_mapper_s,`/dev/mapper/%s')
CONFIG_SCR(command_mdadm,`mdadm --assemble')

/* = Host-Specific Configuration = */
