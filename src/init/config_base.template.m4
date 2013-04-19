divert(-1)
dnl = Base Template =
dnl
dnl This file should be concatenated with the configuration file and run through
dnl m4 to produce config_base.h, which should only be included in config.c (and
dnl only after config.h).
dnl
changecom(@@)
define(`TEMPFILE',`srand')
dnl FIXME: Remember the length of this array and store it in a gi_$1 variable
dnl        for use by the string descrambler so that we don't have to rely on
dnl        NULL termination in XOR'ed strings and we don't have gaping holes of
dnl        padding left in the binary.
define(`CONFIG_SCR',`const char gac_$1[incr(len(`$2'))] = { esyscmd(./scripts/xor.sh -f TEMPFILE() "$2") };
const int gai_$1 = len(`$2');')
define(`CONFIG_RAW',`#define $1 $2')
define(`CONFIG_END',`
#endif /* SCRAMBLES_BASE_H */')

divert(0)
#ifndef SCRAMBLES_BASE_H
#define SCRAMBLES_BASE_H

/* = Key Configuration = */

/* This must occur before ANY strings can be scrambled! */

const char gac_skey[] = { esyscmd(./scripts/xor.sh -f TEMPFILE() -k) };
