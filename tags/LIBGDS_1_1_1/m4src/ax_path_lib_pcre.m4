dnl @synopsis AX_PATH_LIB_PCRE [(A/NA)]
dnl
dnl check for pcre lib and set PCRE_LIBS and PCRE_CFLAGS accordingly.
dnl 
dnl also provide --with-pcre option that may point to the $prefix of
dnl the pcre installation - the macro will check $pcre/include and
dnl $pcre/lib to contain the necessary files.
dnl 
dnl the usual two ACTION-IF-FOUND / ACTION-IF-NOT-FOUND are supported
dnl and they can take advantage of the LIBS/CFLAGS additions.
dnl
dnl @category InstalledPackages
dnl @author Guido Draheim (guidod@dmx.de)
dnl @author Bruno Quoitin (bqu@info.ucl.ac.be)
dnl @version 2005-01-27
dnl @license GPL

AC_DEFUN([AX_PATH_LIB_PCRE],[dnl
AC_MSG_CHECKING([lib pcre])
AC_ARG_WITH(pcre,
  AC_HELP_STRING([--with-pcre[[=prefix]]], [define libpcre path]),,
  with_pcre="")

if test ".$with_pcre" = ".no" ; then
  AC_MSG_RESULT([disabled])
  m4_ifval($2,$2)
else
  AC_MSG_RESULT([(testing)])
  OLDLDFLAGS="$LDFLAGS"
  OLDCPPFLAGS="$CPPFLAGS"

  dnl Check presence of library
  if test ".$with_pcre" != "."; then
    PCRE_TMP_LDFLAGS="-L$with_pcre/lib"
    PCRE_TMP_CFLAGS="-I$with_pcre/include"
  fi
  LDFLAGS="$LDFLAGS $PCRE_TMP_LDFLAGS"
  CPPFLAGS="$CPPFLAGS $PCRE_TMP_CFLAGS"

  AC_CHECK_LIB(pcre, pcre_compile)
  if test "$ac_cv_lib_pcre_pcre_compile" = "yes" ; then
    PCRE_LIBS="-lpcre $PCRE_TMP_LDFLAGS"

    dnl Check presence of header
    dnl (can be found in either include/ or include/pcre/)
    AC_CHECK_HEADERS([pcre.h pcre/pcre.h], [break])
    if test $ac_cv_header_pcre_h != no ||
       test $ac_cv_header_pcre_pcre_h != no; then
      PCRE_CFLAGS=$PCRE_TMP_CFLAGS
      m4_ifval($1,$1)
    else
      m4_ifval($2,$2)
    fi
  else
    m4_ifval($2,$2)
  fi

  CPPFLAGS="$OLDCPPFLAGS"
  LDFLAGS="$OLDLDFLAGS"
fi
AC_SUBST([PCRE_LIBS])
AC_SUBST([PCRE_CFLAGS])
])
