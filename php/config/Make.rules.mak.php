# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
# Specify your C++ compiler. The only value currently supported is VC90.
# Leave unset for auto-detection.
#
#CPP_COMPILER           = VC90

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
PHP_HOME		= C:\php-5.4.20
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
!if "$(PROCESSOR_ARCHITECTURE)" == "AMD64"
THIRDPARTY_HOME	 = $(PROGRAMFILES) (x86)\ZeroC\Ice-$(VERSION)-ThirdParty
!else
THIRDPARTY_HOME	 = $(PROGRAMFILES)\ZeroC\Ice-$(VERSION)-ThirdParty
!endif
!endif


# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

!if "$(USE_BIN_DIST)" != "yes"

#
# Check CPP_COMPILER
#
!if "$(CPP_COMPILER)" == ""

!if "$(VISUALSTUDIOVERSION)" == "11.0"
!error Detected VC110 compiler
!elseif ([cl 2>&1 | findstr "Version\ 15" > nul] == 0)
CPP_COMPILER            = VC90
!elseif ([cl 2>&1 | findstr "Version\ 16" > nul] == 0)
!error Detected VC100 compiler
!elseif ([cl 2>&1 | findstr "Version\ 17" > nul] == 0)
!error Detected VC110 compiler
!else
!error Cannot detect C++ compiler 
!endif

!elseif "$(CPP_COMPILER)" != "VC90"
!error Invalid CPP_COMPILER setting: $(CPP_COMPILER). Must be set to VC90.
!endif

!endif

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

!if exist ($(top_srcdir)\..\cpp\config\Make.rules.msvc)
!include $(top_srcdir)\..\cpp\config\Make.rules.msvc
!else
!include $(top_srcdir)\config\Make.rules.msvc
!endif

libsuff         = $(x64suffix)

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
RCFLAGS		= -D_DEBUG
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
SLICE2PHP		= $(ice_cpp_dir)\bin\slice2php.exe
SLICEPARSERLIB		= $(ice_cpp_dir)\lib\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_cpp_dir)\lib\sliced.lib
!endif
!else
SLICE2PHP		= $(ice_cpp_dir)\bin$(x64suffix)\slice2php.exe
SLICEPARSERLIB		= $(ice_cpp_dir)\lib$(x64suffix)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_cpp_dir)\lib$(x64suffix)\sliced.lib
!endif
!endif
!else
SLICE2PHP		= $(ice_dir)\bin$(x64suffix)\slice2php.exe
SLICEPARSERLIB		= $(ice_dir)\lib$(x64suffix)\slice.lib
!if !exist ("$(SLICEPARSERLIB)")
SLICEPARSERLIB		= $(ice_dir)\lib$(x64suffix)\sliced.lib
!endif
!endif

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .obj .php .res .rc

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.rc.res:
	rc $(RCFLAGS) $<

clean::
	del /q $(TARGETS) *.obj *.bak

all:: $(SRCS) $(TARGETS)

install::
