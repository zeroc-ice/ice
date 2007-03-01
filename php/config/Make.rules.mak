# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

!if "$(ICE_HOME)" == ""
!error ICE_HOME is not defined
!endif

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= C:\IcePHP-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

OPTIMIZE		= yes

#
# Set PHP_HOME to your PHP source directory.
#

PHP_HOME		= C:\php-5.2.1

#
# Set PHP_BIN_HOME to your PHP binary installation directory.
#

PHP_BIN_HOME		= C:\php-5.2.1-Win32

#
# Set STLPORT_HOME to your STLPort installation directory.
#

STLPORT_HOME            = C:\Ice-$(VERSION)-ThirdParty-VC60

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 3.2.0
SOVERSION		= 32
bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib

!if exist ($(top_srcdir)\slice)
slicedir		= $(top_srcdir)\slice
!else
slicedir		= $(ICE_HOME)\slice
!endif

install_libdir		= $(prefix)\bin
install_libdir		= $(prefix)\lib
install_slicedir        = $(prefix)\slice

THIRDPARTY_HOME         = $(STLPORT_HOME)
CPP_COMPILER		= VC60

!if exist ($(top_srcdir)\config\Make.rules.msvc)
!include $(top_srcdir)\config\Make.rules.msvc
!else
!include $(ICE_HOME)\config\Make.rules.msvc
!endif

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
!endif

ICE_LIBS		= ice$(LIBSUFFIX).lib iceutil$(LIBSUFFIX).lib slice$(LIBSUFFIX).lib

ICE_CPPFLAGS		= -I"$(ICE_HOME)\include"
ICE_LDFLAGS		= /LIBPATH:"$(ICE_HOME)\lib"

PHP_CPPFLAGS		= -I"$(PHP_HOME)" -I"$(PHP_HOME)\main" -I"$(PHP_HOME)\TSRM" -I"$(PHP_HOME)\Zend" -DPHP_WIN32 -DZEND_WIN32 -DZEND_DEBUG=0 -DZTS
PHP_LDFLAGS		= /LIBPATH:"$(PHP_BIN_HOME)\dev"
PHP_LIBS		= php5ts.lib

ICECPPFLAGS		= -I$(slicedir)

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .obj .rb

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

clean::
	del /q $(TARGETS) *.obj *.bak

all:: $(SRCS) $(TARGETS)

install::
