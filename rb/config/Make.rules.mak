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

prefix			= C:\IceRuby-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

OPTIMIZE		= yes

#
# Specify your C++ compiler. Supported values are:
# VC60
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER            = VC60
!endif

#
# Set RUBY_HOME to your Ruby installation directory.
#

RUBY_HOME		= C:\ruby

#
# Set STLPORT_HOME to your STLPort installation directory.
#
STLPORT_HOME            = C:\Ice-$(VERSION)-ThirdParty-VC60

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION_MAJOR           = 3
VERSION_MINOR           = 2
VERSION_PATCH           = 0
VERSION			= $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
SOVERSION		= $(VERSION_MAJOR)$(VERSION_MINOR)
libdir			= $(top_srcdir)\lib
rubydir			= $(top_srcdir)\ruby

!if exist ($(top_srcdir)\slice)
slicedir		= $(top_srcdir)\slice
!else
slicedir		= $(ICE_HOME)\slice
!endif

install_libdir		= $(prefix)\lib
install_slicedir        = $(prefix)\slice
install_rubydir		= $(prefix)\ruby

!include $(top_srcdir)\config\Make.rules.msvc

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
!endif

ICE_LIBS		= ice$(LIBSUFFIX).lib iceutil$(LIBSUFFIX).lib slice$(LIBSUFFIX).lib

ICE_CPPFLAGS		= -I"$(ICE_HOME)\include"
ICE_LDFLAGS		= /LIBPATH:"$(ICE_HOME)\lib"

RUBY_CPPFLAGS		= -I"$(RUBY_HOME)\lib\ruby\1.8\i386-mswin32"
RUBY_LDFLAGS		= /LIBPATH:"$(RUBY_HOME)\lib"
RUBY_LIBS		= msvcrt-ruby18.lib

ICECPPFLAGS		= -I$(slicedir)
SLICE2RBFLAGS		= $(ICECPPFLAGS)

SLICE2RB		= $(ICE_HOME)\bin\slice2rb.exe

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .obj .rb

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

clean::
	del /q $(TARGETS) core *.obj *.bak

all:: $(SRCS) $(TARGETS)

install::
