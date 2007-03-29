# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
# Define if you want to build the Ice-E slice compilers with
# static libraries.
#
#STATICLIBS             = yes

#
# Specify your C++ compiler. Supported values are:
# VC60, VC71, VC80, VC80_EXPRESS, BCC2006 
#
!if "$(CPP_COMPILER)" == ""
CPP_COMPILER		= VC80
!endif

#
# Borland C++Builder 2006 home directory. Used if CPP_COMPILER
# is set to BCC2006. Change if different from default.
#
!if "$(BCB)" == ""
BCB		= C:\Program Files\Borland\BDS\4.0	
!endif

#
# If third party libraries are not installed in the default location
# or THIRDPARTY_HOME is not set in your environment variables then
# change the following setting to reflect the installation location.
#
!if "$(CPP_COMPILER)" == "BCC2006"
TPH_EXT		= BCC
!elseif "$(CPP_COMPILER)" == "VC80_EXPRESS"
TPH_EXT		= VC80
!else
TPH_EXT		= $(CPP_COMPILER)
!endif

!if "$(THIRDPARTY_HOME)" == ""
THIRDPARTY_HOME		= C:\Ice-$(VERSION)-ThirdParty-$(TPH_EXT)

!if "$(AS)" == "ml64"
THIRDPARTY_HOME		= $(THIRDPARTY_HOME)-x64
!endif

!endif

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
VERSION			= 3.2.0
SOVERSION		= 32
bindir			= $(top_srcdir)\bin
libdir			= $(top_srcdir)\lib
includedir		= $(top_srcdir)\include
slicedir		= $(top_srcdir)\slice

install_bindir		= $(prefix)\bin

install_includedir	= $(prefix)\include
install_slicedir	= $(prefix)\slice
install_schemadir	= $(prefix)\schema
install_docdir		= $(prefix)\doc

OBJEXT			= .obj

SETARGV			= setargv.obj

!if "$(CPP_COMPILER)" == "BCC2006"
!include 	$(top_srcdir)/config/Make.rules.bcc
!else
!include 	$(top_srcdir)/config/Make.rules.msvc
!endif

install_libdir	  = $(prefix)\lib
libsubdir	  = lib

!if "$(OPTIMIZE)" != "yes"
LIBSUFFIX	= $(LIBSUFFIX)d
!endif

OPENSSL_LIBS            = ssleay32.lib libeay32.lib
EXPAT_LIBS              = libexpat.lib

CPPFLAGS		= $(CPPFLAGS) -I$(includedir)
ICECPPFLAGS		= -I$(slicedir)
SLICE2CPPFLAGS		= $(ICECPPFLAGS)

LDFLAGS			= $(LDFLAGS) $(LDPLATFORMFLAGS) $(CXXFLAGS)

SLICEPARSERLIB		= $(libdir)\slice$(LIBSUFFIX).lib
SLICE2CPP		= $(bindir)\slice2cpp.exe
SLICE2XSD		= $(bindir)\slice2xsd.exe
SLICE2FREEZE		= $(bindir)\slice2freeze.exe
SLICE2DOCBOOK		= $(bindir)\slice2docbook.exe

EVERYTHING		= all clean install

.SUFFIXES:
.SUFFIXES:		.ice .cpp .c .obj

.cpp.obj::
	$(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

.c.obj:
	$(CC) /c $(CPPFLAGS) $(CFLAGS) $<


{$(SDIR)\}.ice{$(HDIR)}.h:
	del /q $(HDIR)\$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $<
	move $(*F).h $(HDIR)

.ice.cpp:
	del /q $(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(*F).ice

all:: $(SRCS) $(TARGETS)

!if "$(TARGETS)" != ""

clean::
	-del /q $(TARGETS)

!endif

clean::
	-del /q *.obj *.bak *.ilk *.exp *.pdb *.tds

install::
