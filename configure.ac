dnl =================================================================
dnl configure.in for libgds
dnl
dnl @author Bruno Quoitin (bruno.quoitin@umons.ac.be)
dnl $Id$
dnl =================================================================

m4_include([version.m4])
AC_INIT([libgds], [VERSION_NUMBER], [bruno.quoitin@umons.ac.be])
AC_CONFIG_SRCDIR(src/libgds/fifo.h)
AC_PREREQ(2.57)
AC_PREFIX_DEFAULT(/usr/local)

dnl Version setting up for Automake
AM_INIT_AUTOMAKE([no-define])
AC_CONFIG_HEADER([src/config.h src/libgds/libgds-config.h])
dnl AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

CFLAGS="-Wall -O2 --std=gnu99 $CFLAGS"
#CFLAGS="-Wall -Werror -O2 --std=gnu99 $CFLAGS"
#CFLAGS="-Wall -Werror -O0 --std=gnu99 $CFLAGS"

dnl GNU package
AC_DEFINE([_GNU_SOURCE], 1,
          [Define _GNU_SOURCE so that we get all necessary prototypes])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER(limits.h)
AC_CHECK_HEADER(inttypes.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

dnl Now arrange the build libtool
AC_PROG_LIBTOOL
AC_SUBST(LIBGDS_LT_RELEASE, [VERSION_NUMBER])

AC_CHECK_FUNCS(strcspn strsep strdup vasprintf)

dnl Test for POSIX thread
#ACX_PTHREAD([pthread_ok=yes], [pthread_ok=no])
#AC_MSG_NOTICE($pthread_ok)
#if test "x$pthread_ok" == "xyes"; then
#  LIBS="$LIBS $PTHREAD_LIBS"
#  CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
#  CPPFLAGS="$CPPFLAGS $PTHREAD_CFLAGS"
#  AC_MSG_NOTICE($LIBS);
#  AC_MSG_NOTICE($CFLAGS);
#  AC_MSG_NOTICE($CPPFLAGS);
#else
#  AC_MSG_ERROR([pthreads required to compile libgds]);
#fi

dnl Test for libxml2 usability (library + headers)
AC_ARG_ENABLE(xml,
              AC_HELP_STRING([--enable-xml],
                             [enable XML support]),
              gds_config_xml="$enableval",
              )
#if test "x$gds_config_xml" = "xyes"; then
#  dnl Check for existence of 'xml2-config'
#  AM_PATH_XML2(2.0.0,
#                [ LIBS="${LIBS} ${XML_LIBS}" ]
#		[ CPPFLAGS="${CPPFLAGS} ${XML_CFLAGS} ${XML_CPPFLAGS}" ],
#		)
#   AC_CHECK_LIB([xml2], [xmlNodeGetContent], [xml_ok=yes], [xml_ok=no])
#   if test "x$xml_ok" != "xyes"; then
#      AC_MSG_ERROR([libxml2 is needed to compile xml.* of libgds])
#   else
#      dnl Check for headers
#      AC_CHECK_HEADER([libxml/tree.h], [xml_header_ok=yes], [xml_header_ok=no])
#      if test "x$xml_header_ok" != "xyes"; then
#         AC_MSG_ERROR([libxml2 headers are needed to compile xml.* of libgds])
#      else
#         AC_DEFINE(HAVE_LIBXML, 1, [Define to 1 if you have the 'xml2' library])
#      fi
#   fi
#fi

dnl Enable the memory debugging
AC_ARG_ENABLE(memory-debug,
    AC_HELP_STRING([--enable-memory-debug],
      [helps to track down memory allocation errors]),
    gds_config_memory_debug="$enableval",
)
if test "x$gds_config_memory_debug" = "xyes"; then
  CFLAGS="$CFLAGS -DMEMORY_DEBUG"
fi

dnl Enable doxygen generation and inclusion in "make dist"
AC_PATH_PROG(DOXYGEN, doxygen, "no")
if test "x$DOXYGEN" != "xno"; then
  AC_CHECK_PROG(HAVE_DOXYGEN, doxygen, "yes", "no")
  #dnl we found a doxygen but it is not running correctly
  if test "x$HAVE_DOXYGEN" = "xno"; then
    AC_MSG_WARN(["Doxygen is not correctly installed on this system."])
  fi
else
  AC_MSG_WARN(["Doxygen is not installed on this system."])
fi
AM_CONDITIONAL(WITH_DOXYGEN, [test "x$HAVE_DOXYGEN" = "xyes"])

AC_OUTPUT(Makefile src/Makefile src/libgds/Makefile src/check/Makefile doc/Makefile libgds.pc libgds.spec Doxyfile)

if test "x$gds_config_xml" = "xyes"; then
  config_xml_msg="yes"
else
  config_xml_msg="no"
fi

if test "x$gds_config_memory_debug" = "xyes"; then
  config_memory_debug_msg="yes"
else
  config_memory_debug_msg="no"
fi

case $host in
  *-*-cygwin*)
    test_cygwin="yes"
    AC_DEFINE([CYGWIN], 1, [Define to 1 if the system is cygwin])
    ;;
  *)
    test_cygwin="no"
    ;;
esac

echo ""
echo "Libgds package has been configured with the following optional capabilities :"
echo "                              XML : $config_xml_msg"
echo "                     Memory Debug : $config_memory_debug_msg"
