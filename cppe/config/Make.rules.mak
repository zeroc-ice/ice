# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Select an installation base directory. The directory will be created
# if it does not exist.
#
prefix			= C:\IceE-$(VERSION)

#
# Define OPTIMIZE_SIZE as yes if you want to build with minimal size.
# Define OPTIMIZE_SPEED as yes if you want to build with maximum speed.
# These options are mutually exclusive.
# If neither is set, IceE is built with debug information.
#
#OPTIMIZE_SIZE		= yes
#OPTIMIZE_SPEED		= yes

#
# Specify your C++ compiler. Supported values are
# VC80 or VC80_EXPRESS
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER            = VC80
!endif

#
# Define STATICLIBS as yes if you want to build with static libraries.
# Otherwise IceE is built with dynamic libraries.
#
#STATICLIBS             = yes

#
# If building for an WinCE embedded device with VS2005 set SMART_DEVICE.
# Supported options are PocketPC2003 and Smartphone2003.
#
#SMART_DEVICE	= PocketPC2003

#
# For VC80 and VC80 Express it is necessary to set the location of the
# manifest tool. This must be the 6.x version of mt.exe, not the 5.x
# version!
#
# For VC80 Express mt.exe 6.x is provided by the Windows Platform SDK.
# It is necessary to set the location of the Platform SDK through the
# PDK_HOME environment variable (see INSTALL.WINDOWS for details).
#
!if "$(CPP_COMPILER)" == "VC80"
MT = "$(VS80COMNTOOLS)bin\mt.exe"
!elseif "$(CPP_COMPILER)" == "VC80_EXPRESS"
MT = "$(PDK_HOME)\bin\mt.exe"
!endif

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 1.2.0
SOVERSION		= 12
bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib
includedir		= $(top_srcdir)\include
slicedir		= $(top_srcdir)\slice

install_bindir		= $(prefix)\bin

install_includedir	= $(prefix)\include
install_slicedir	= $(prefix)\slice
install_schemadir	= $(prefix)\schema
install_docdir		= $(prefix)\doc

INSTALL			= copy
INSTALL_PROGRAM		= $(INSTALL)
INSTALL_LIBRARY		= $(INSTALL)
INSTALL_DATA		= $(INSTALL)

OBJEXT			= .obj

!include 	$(top_srcdir)/config/Make.rules.msvc

install_libdir	  = $(prefix)\lib
libsubdir	  = lib

!if "$(OPTIMIZE_SPEED)" != "yes" & "$(OPTIMIZE_SIZE)" != "yes"
LIBSUFFIX	= $(LIBSUFFIX)d
!endif

CPPFLAGS		= $(CPPFLAGS) -I$(includedir)
ICECPPFLAGS		= -I$(slicedir)
SLICE2CPPEFLAGS		= $(ICECPPFLAGS)

LDFLAGS			= $(LDFLAGS) $(LDPLATFORMFLAGS) $(CXXFLAGS)

SLICE2CPP		= slice2cppe.exe

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.ice .cpp .c .obj

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.c.obj:
	$(CC) /c $(CPPFLAGS) $(CFLAGS) $<


{$(SDIR)\}.ice{$(HDIR)}.h:
	del /q $(HDIR)\$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPEFLAGS) $<
	move $(*F).h $(HDIR)

.ice.cpp:
	del /q $(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPEFLAGS) $(*F).ice

all:: $(SRCS) $(TARGETS)

!if "$(TARGETS)" != ""

clean::
	-del /q $(TARGETS)

!endif

clean::
	-del /q *.obj *.bak *.ilk *.exp *.pdb

install::
