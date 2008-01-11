# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

!if "$(ICE_HOME)" == ""
ICE_DIR		= $(top_srcdir)\..
USE_SRC_DIR	= 1
!else
ICE_DIR 	= $(ICE_HOME)
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
!if "$(PHP_HOME)" == ""
PHP_HOME		= C:\php-5.2.1
!endif

#
# Set PHP_BIN_HOME to your PHP binary installation directory.
#
!if "$(PHP_BIN_HOME)" == ""
PHP_BIN_HOME		= C:\php-5.2.1-Win32
!endif

#
# Set STLPORT_HOME to your STLPort installation directory.
#
!if "$(STLPORT_HOME)" == ""
STLPORT_HOME            = C:\Ice-$(VERSION)-ThirdParty-VC60
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 3.3.0
SOVERSION		= 33
bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib

!if exist ("$(ICE_DIR)\slice")
slicedir 		= $(ICE_DIR)\slice
!else
slicedir                = $(ICE_DIR)\..\slice
!endif

install_libdir		= $(prefix)\bin
install_libdir		= $(prefix)\lib
install_slicedir        = $(prefix)\slice

THIRDPARTY_HOME         = $(STLPORT_HOME)
CPP_COMPILER		= VC60

!if exist ($(ICE_DIR)\config\Make.rules.msvc)
!include $(ICE_DIR)\config\Make.rules.msvc
!else
!include $(ICE_DIR)\cpp\config\Make.rules.msvc
!endif

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
!endif

ICE_LIBS		= ice$(LIBSUFFIX).lib iceutil$(LIBSUFFIX).lib slice$(LIBSUFFIX).lib

!if "$(USE_SRC_DIR)" == "1"
ICE_CPPFLAGS		= -I"$(ICE_DIR)\cpp\include"
ICE_LDFLAGS		= /LIBPATH:"$(ICE_DIR)\cpp\lib"
!else
ICE_CPPFLAGS		= -I"$(ICE_DIR)\include"
ICE_LDFLAGS		= /LIBPATH:"$(ICE_DIR)\lib"
!endif

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
