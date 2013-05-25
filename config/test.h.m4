
/* == Disks == */

dnl CONFIG_SCR(
dnl    `md_arrays',`md1</dev/sda1|/dev/sdb1>md2</dev/sda2|/dev/sdb2>'
dnl )
CONFIG_SCR(`md_arrays',`md2</dev/sda2|/dev/sdb2>')
CONFIG_SCR(`luks_vols',`crypt-root</dev/md2>')

/* == Prompt == */

CONFIG_RAW(`CONFIG_MAX_ATTEMPTS',4)

/* == SSH Prompt == */

dnl = Examples =
dnl == VLAN tagged 1 on eth0 ==
dnl CONFIG_RAW(`NET',1)
dnl CONFIG_SCR(`net_if',`eth0.1')
dnl CONFIG_SCR(`net_ip',`192.168.76.9')
dnl CONFIG_SCR(`ssh_port',`1220')
dnl CONFIG_RAW(`VLAN',1)
dnl CONFIG_RAW(`VLAN_VID',1)
dnl CONFIG_SCR(`net_vlan_if',`eth4')

CONFIG_RAW(`NET',1)
CONFIG_SCR(`net_if',`eth0')
CONFIG_SCR(`net_ip',`192.168.76.9')
CONFIG_RAW(`NET_DNS',`"192.168.76.3"')
CONFIG_RAW(`NET_GATEWAY',`"192.168.76.2"')
CONFIG_RAW(`SSH_PORT',`"1220"')
dnl CONFIG_RAW(`TOR',1)
dnl CONFIG_RAW(`DHCP',1)

/* == Console == */

CONFIG_RAW(`CONSOLE',1)
CONFIG_SCR(`sys_console_pw',`console')

/* == Other == */

CONFIG_RAW(`COPY_FIRMWARE',`"lib64/firmware/rtl_nic/rtl8168e-1.fw lib64/firmware/rtl_nic/rtl8168e-2.fw lib64/firmware/rtl_nic/rtl8168e-3.fw"');

CONFIG_END()

