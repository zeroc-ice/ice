# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

prefix			= C:\IcePy-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with optimization.
# Otherwise the Ice extension is built with debug information.
#

#OPTIMIZE		= yes

#
# Set PYTHON_HOME to your Python installation directory.
#

PYTHON_HOME		= C:\Python24

#
# Define if using STLPort. Required if using MSVC++ 6.0.
#
STLPORT_HOME            = C:\Ice-$(VERSION)-ThirdParty

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION_MAJOR           = 3
VERSION_MINOR           = 2
VERSION_PATCH           = 0
VERSION			= $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)
SOVERSION		= $(VERSION_MAJOR)$(VERSION_MINOR)
libdir			= $(top_srcdir)\python

install_slicedir	= $(prefix)\slice
install_pythondir	= $(prefix)\python

!if exist ($(top_srcdir)\config\Make.rules.msvc)
configdir		= $(top_srcdir)\config
!else
configdir		= $(ICE_HOME)\config
!endif

!if "$(STLPORT_HOME)" != ""
CPPFLAGS		= -I"$(STLPORT_HOME)\include\stlport"
LDFLAGS			= /LIBPATH:"$(STLPORT_HOME)\lib"
!endif

!include $(configdir)\Make.rules.msvc

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX       = $(LIBSUFFIX)d
PYLIBSUFFIX     = _$(LIBSUFFIX)
!endif

ICE_LIBS		= ice$(LIBSUFFIX).lib iceutil$(LIBSUFFIX).lib slice$(LIBSUFFIX).lib

ICE_CPPFLAGS		= -I"$(ICE_HOME)\include"
ICE_LDFLAGS		= /LIBPATH:"$(ICE_HOME)\lib"

!if exist ($(top_srcdir)\slice)
slicedir		= $(top_srcdir)\slice
!else
slicedir		= $(ICE_HOME)\slice
!endif

PYTHON_CPPFLAGS		= -I"$(PYTHON_HOME)\include"
PYTHON_LDFLAGS		= /LIBPATH:"$(PYTHON_HOME)\libs"

install_libdir		= $(prefix)\python
libsubdir		= lib

ICECPPFLAGS		= -I$(slicedir)
SLICE2PYFLAGS		= $(ICECPPFLAGS)

SLICE2PY		= $(ICE_HOME)\bin\slice2py.exe

EVERYTHING		= all depend clean install

.SUFFIXES:
.SUFFIXES:		.cpp .obj .py

all:: $(SRCS)

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

clean::
	del /q $(TARGETS) core *.obj *.pyc *.bak

all:: $(SRCS) $(TARGETS)

install::
