dnl
dnl Contains the following macros
dnl   JAPHAR_GREP_CFLAGS(flag, cmd_if_missing, cmd_if_present)
dnl

dnl JAPHAR_GREP_CFLAGS(flag, cmd_if_missing, cmd_if_present)
AC_DEFUN([JAPHAR_GREP_CFLAGS],
[case "$CFLAGS" in
"$1" | "$1 "* | *" $1" | *" $1 "* )
  ifelse($#, 3, [$3], [:])
  ;;
*)
  $2
  ;;
esac
])
