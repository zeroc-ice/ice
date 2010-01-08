# **********************************************************************
#
# Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#

prefix			= C:\Ice-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

OPTIMIZE		= yes

#
# Specify your C++ compiler. Supported values are:
# VC60, VC90, VC90_EXPRESS
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER            = VC60
!endif

#
# Determines whether the extension uses PHP namespaces (requires
# PHP 5.3 or later).
#
!if "$(USE_NAMESPACES)" == ""
USE_NAMESPACES		= no
!endif

#
# Set PHP_HOME to your PHP source directory.
#
!if "$(PHP_HOME)" == ""
PHP_HOME		= C:\php-5.3.1
!endif

#
# Set PHP_BIN_HOME to your PHP binary installation directory.
#
!if "$(PHP_BIN_HOME)" == ""
PHP_BIN_HOME		= C:\Program Files\PHP
!endif

#
# Set PHP_ZTS to "no" (or comment it out) to disable Zend Thread Safety.
#
!if "$(PHP_ZTS)" == ""
PHP_ZTS			= yes
!endif

#
# If third party libraries are not installed in the default location
# or THIRDPARTY_HOME is not set in your environment variables then
# change the following setting to reflect the installation location.
#
!if "$(THIRDPARTY_HOME)" == ""
!if "$(PROCESSOR_ARCHITECTURE)" == "AMD64" || "$(PROCESSOR_ARCHITECTUREW6432)" == "AMD64"
THIRDPARTY_HOME	 = $(PROGRAMFILES) (x86)\ZeroC\Ice-$(VERSION)-ThirdParty
!else
THIRDPARTY_HOME	 = $(PROGRAMFILES)\ZeroC\Ice-$(VERSION)-ThirdParty
!endif
!endif

#
# STLPort is required if using MSVC++ 6.0. Change if STLPort
# is located in a different location.
#
!if "$(CPP_COMPILER)" == "VC60" && "$(STLPORT_HOME)" == ""
STLPORT_HOME            = $(THIRDPARTY_HOME)
!endif


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

#
# Common definitions
#
ice_language     = php
ice_require_cpp  = yes
slice_translator = slice2php.exe

!if exist ($(top_srcdir)\..\config\Make.common.rules.mak)
!include $(top_srcdir)\..\config\Make.common.rules.mak
!else
!include $(top_srcdir)\config\Make.common.rules.mak
!endif

libdir			= $(top_srcdir)\lib
install_phpdir		= $(prefix)\php
install_libdir		= $(prefix)\php

!if "$(CPP_COMPILER)" != "VC60" && "$(CPP_COMPILER)" != "VC90" && "$(CPP_COMPILER)" != "VC90_EXPRESS"
!error Invalid setting for CPP_COMPILER: $(CPP_COMPILER)
!endif

!if "$(ice_src_dist)" != ""
!include $(top_srcdir)\..\cpp\config\Make.rules.msvc
!else
!include $(top_srcdir)\config\Make.rules.msvc
!endif

!if "$(CPP_COMPILER)" == "VC60"
libsuff         = \vc6
!else
libsuff         = $(x64suffix)
!endif

!if "$(ice_src_dist)" != ""
!if "$(STLPORT_HOME)" != ""
CPPFLAGS        = -I"$(STLPORT_HOME)\include\stlport" $(CPPFLAGS)
LDFLAGS         = /LIBPATH:"$(STLPORT_HOME)\lib$(libsuff)" $(LDFLAGS)
!endif
!else
!if "$(CPP_COMPILER)" == "VC60"
CPPFLAGS        = -I"$(ice_dir)\include\stlport" $(CPPFLAGS)
!endif
!endif

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
!endif

ICE_LIBS		= ice$(LIBSUFFIX).lib iceutil$(LIBSUFFIX).lib slice$(LIBSUFFIX).lib

!if "$(ice_src_dist)" != ""
ICE_CPPFLAGS            = -I"$(ice_cpp_dir)\include"
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
ICE_LDFLAGS             = /LIBPATH:"$(ice_cpp_dir)\lib"
!else
ICE_LDFLAGS             = /LIBPATH:"$(ice_cpp_dir)\lib$(libsuff)"
!endif
!else
ICE_CPPFLAGS            = -I"$(ice_dir)\include"
ICE_LDFLAGS             = /LIBPATH:"$(ice_dir)\lib$(libsuff)"
!endif

slicedir                = $(ice_dir)\slice

!if "$(PHP_ZTS)" == "yes"
PHP_LIB_PREFIX		= php5ts
!else
PHP_LIB_PREFIX		= php5
!endif

!if "$(OPTIMIZE)" != "yes"
PHP_LDFLAGS		= /LIBPATH:"$(PHP_BIN_HOME)"
PHP_LIBS		= $(PHP_LIB_PREFIX)_debug.lib
PHP_ZEND_DEBUG		= 1
!else
PHP_LDFLAGS		= /LIBPATH:"$(PHP_BIN_HOME)\dev"
PHP_LIBS		= $(PHP_LIB_PREFIX).lib
PHP_ZEND_DEBUG		= 0
!endif

PHP_CPPFLAGS		= -I"$(PHP_HOME)" -I"$(PHP_HOME)\main" -I"$(PHP_HOME)\TSRM" -I"$(PHP_HOME)\Zend" -DPHP_WIN32 -DZEND_WIN32 -DZEND_DEBUG=$(PHP_ZEND_DEBUG) -DWIN32
!if "$(PHP_ZTS)" == "yes"
PHP_CPPFLAGS		= $(PHP_CPPFLAGS) -DZTS
!endif

ICECPPFLAGS		= -I"$(slicedir)"
SLICE2PHPFLAGS		= $(ICECPPFLAGS)

!if "$(USE_NAMESPACES)" == "yes"
CPPFLAGS		= $(CPPFLAGS) -DICEPHP_USE_NAMESPACES
SLICE2PHPFLAGS		= $(SLICE2PHPFLAGS) -n
!endif

!if "$(ice_src_dist)" != ""
!if "$(ice_cpp_dir)" == "$(ice_dir)\cpp"
SLICE2PHP               = $(ice_cpp_dir)\bin\slice2php.exe
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_cpp_dir)\lib\sliced.lib
!endif
!else
SLICE2PHP               = $(ice_cpp_dir)\bin$(x64suffix)\slice2php.exe
SLICEPARSERLIB          = $(ice_cpp_dir)\lib$(libsuff)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_cpp_dir)\lib$(libsuff)\sliced.lib
!endif
!endif
!else
SLICE2PHP               = $(ice_dir)\bin$(x64suffix)\slice2php.exe
SLICEPARSERLIB          = $(ice_dir)\lib$(libsuff)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB          = $(ice_dir)\lib$(libsuff)\sliced.lib
!endif
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .obj .php

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

clean::
	del /q $(TARGETS) *.obj *.bak

all:: $(SRCS) $(TARGETS)

install::
