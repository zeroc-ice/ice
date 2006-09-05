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
prefix			= C:\Ice-$(VERSION)

#
# Define OPTIMIZE as yes if you want to build with
# optimization. Otherwise Ice is build with debug information.
#
#OPTIMIZE		= yes


#
# Borland C++Builder 2006 home directory. Define if building with 
# C++ Builder. Change if different from default.
#
#BORLAND_HOME		= C:\Program Files\Borland\BDS\4.0	

#
# If third party libraries are not installed in the default location
# change the following setting to reflect the installation location.
#
#THIRDPARTY_HOME		= C:\Ice-$(VERSION)-ThirdParty
THIRDPARTY_HOME		= c:\src\packages_win32

#
# Define if using STLPort. Required if using MSVC++ 6.0.
#
STLPORT_HOME		= $(THIRDPARTY_HOME)

# ----------------------------------------------------------------------
# Don't change anything below this line!
# ----------------------------------------------------------------------

SHELL			= /bin/sh
VERSION			= 3.1.0
SOVERSION		= 31
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

!if "$(BORLAND_HOME)" == ""
!include 	$(top_srcdir)/config/Make.rules.msvc
!else
!include 	$(top_srcdir)/config/Make.rules.bcc
!endif

install_libdir	  = $(prefix)\lib
libsubdir	  = lib

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX	= $(LIBSUFFIX)d
!endif

OPENSSL_LIBS            = ssleay32.lib libeay32.lib
BZIP2_LIBS              = libbz2.lib
DB_LIBS                 = libdb43.lib
EXPAT_LIBS              = libexpat.lib

CPPFLAGS		= $(CPPFLAGS) -I$(includedir)
ICECPPFLAGS		= -I$(slicedir)
SLICE2CPPFLAGS		= $(ICECPPFLAGS)

LDFLAGS			= $(LDFLAGS) $(LDPLATFORMFLAGS) $(CXXFLAGS)

SLICEPARSERLIB		= $(top_srcdir)\lib\slice$(LIBSUFFIX).lib
SLICE2CPP		= $(bindir)\slice2cpp.exe
SLICE2XSD		= $(bindir)\slice2xsd.exe
SLICE2FREEZE		= $(bindir)\slice2freeze.exe
SLICE2DOCBOOK		= $(bindir)\slice2docbook.exe

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.cpp .c .obj .ice

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.c.obj:
	$(CC) /c $(CPPFLAGS) $(CFLAGS) $<


{$(SDIR)\}.ice{$(HDIR)}.h:
	del /q $(HDIR)\$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $<
	move $(*F).h $(HDIR)

.ice.h:
	del /q $(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(*F).ice

all:: $(SRCS) $(TARGETS)

!if "$(TARGETS)" != ""

clean::
	-del /q $(TARGETS)

!endif

clean::
	-del /q core *.obj *.bak *.ilk *.exp *.pdb *.tds

install::
