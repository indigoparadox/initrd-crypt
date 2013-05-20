divert(-1)
dnl = Configuration Settings =
dnl
divert(0)
/* = Generic Configuration = */

CONFIG_SCR(
   `sys_fs_mount',
   `/sys<none|sysfs>/proc<none|proc>/dev/pts<none|devpts>'
)
CONFIG_SCR(sys_fs_types,`ext4|ext3|ext2|xfs|reiserfs')
CONFIG_SCR(sys_mpoint_root,`/mnt/root')
CONFIG_SCR(sys_path_mapper,`/dev/mapper')
CONFIG_SCR(sys_path_mapper_s,`/dev/mapper/%s')
CONFIG_SCR(sys_path_sshpid,`/var/run/dropbear.pid')
CONFIG_SCR(sys_path_dhcppid,`/var/run/udhcpc.pid')
CONFIG_SCR(command_prompt,`init')
CONFIG_SCR(command_mdadm,`mdadm --assemble')
dnl CONFIG_SCR(command_switch_root,`switch_root|/mnt/root|/sbin/init')
CONFIG_SCR(command_switch_root,`/sbin/init')
dnl CONFIG_SCR(command_ssh,`/sbin/dropbear|-s|-j|-k|-p|22')
CONFIG_SCR(command_ssh,`/sbin/dropbear -s -j -k -P /var/run/dropbear.pid -p %s')
CONFIG_SCR(command_tor,`/bin/tor|--quiet|-f|/etc/torrc')
CONFIG_SCR(command_dhcp,`/bin/udhcpc -t 5 -q -p /var/run/udhcpc.pid -s /bin/simple.script')

/* = Host-Specific Configuration = */
