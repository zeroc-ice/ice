dnl $Id$
dnl config.m4 for extension ice

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(ice, for Ice support,
[  --with-ice              Include Ice support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(ice, whether to enable Ice support,
dnl Make sure that the comment is aligned:
dnl [  --enable-ice            Enable Ice support])

if test "$PHP_ICE" != "no"; then
  dnl Write more examples of tests here...

  # --with-ice -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
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
    AC_MSG_ERROR([Please reinstall the Ice distribution])
  fi

  # --with-ice -> add include path
  PHP_ADD_INCLUDE($ICE_DIR/include)

  dnl # --with-ice -> check for lib and symbol presence
  dnl LIBNAME=Ice # you may want to change this
  dnl LIBSYMBOL=ice # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $ICE_DIR/lib, ICE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_ICELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong Ice lib version or lib not found])
  dnl ],[
  dnl   -L$ICE_DIR/lib -lm -ldl
  dnl ])
  dnl
  dnl PHP_SUBST(ICE_SHARED_LIBADD)

  PHP_REQUIRE_CXX()
  PHP_ADD_LIBPATH($ICE_DIR/lib, ICE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(Ice, 1, ICE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(Slice, 1, ICE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(IceUtil, 1, ICE_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, 1, ICE_SHARED_LIBADD)
  PHP_NEW_EXTENSION(ice, ice.cpp communicator.cpp exception.cpp marshal.cpp proxy.cpp slice.cpp struct.cpp util.cpp, $ext_shared)
fi
