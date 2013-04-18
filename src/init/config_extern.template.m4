divert(-1)
dnl = Base Template =
dnl
dnl This file should be concatenated with the configuration file and run through
dnl m4 to produce config_extern.h, which should only be included in files OTHER
dnl than config.c that need access to configuration constants/scrambled strings
dnl (and only after config.h).
dnl
define(`TEMPFILE',`srand')
define(`CONFIG_SCR',`extern const char* gac_$1;')
define(`CONFIG_RAW',`#define $1 $2')
define(`CONFIG_END',`
#endif /* SCRAMBLES_EXTERN_H */')

divert(0)
#ifndef SCRAMBLES_EXTERN_H
#define SCRAMBLES_EXTERN_H
