dnl $Id$
dnl config.m4 for extension ice

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

PHP_ARG_WITH(ice, for Ice support,
[  --with-ice              Include Ice support])

if test "$PHP_ICE" != "no"; then

  SEARCH_PATH="/usr/local /usr"
  SEARCH_FOR="/include/Ice/Ice.h"
  if test -r $PHP_ICE/$SEARCH_FOR; then # path given as parameter
    ICE_DIR=$PHP_ICE
  else # search default path list
    AC_MSG_CHECKING([for Ice files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        ICE_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi
  
  if test -z "$ICE_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the Ice distribution from http://www.zeroc.com])
  fi

  PHP_ADD_INCLUDE($ICE_DIR/include)

  PHP_REQUIRE_CXX()
  PHP_ADD_LIBPATH($ICE_DIR/lib, ICE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(Ice, 1, ICE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(Slice, 1, ICE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(IceUtil, 1, ICE_SHARED_LIBADD)
  case $host_os in
  solaris*)
    if test "$GXX" != "yes"; then
      PHP_ADD_LIBRARY(Cstd, 1, ICE_SHARED_LIBADD)
      PHP_ADD_LIBRARY(Crun, 1, ICE_SHARED_LIBADD)
    fi
  ;;
  esac

  sources="ice.cpp \
           communicator.cpp \
           marshal.cpp \
           profile.cpp \
           proxy.cpp \
           util.cpp"

  PHP_SUBST(ICE_SHARED_LIBADD)
  PHP_NEW_EXTENSION(ice, $sources, $ext_shared,,,yes)
fi
