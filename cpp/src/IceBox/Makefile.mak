# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icebox$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icebox$(SOVERSION)$(LIBSUFFIX).dll

SERVER		= $(top_srcdir)\bin\icebox$(LIBSUFFIX).exe
ADMIN		= $(top_srcdir)\bin\iceboxadmin.exe

!ifdef BUILD_UTILS

TARGETS         = $(SERVER) $(ADMIN)

!else

TARGETS         = $(LIBNAME) $(DLLNAME)

!endif

OBJS		= IceBox.obj \
		  Exception.obj

SOBJS		= ServiceManagerI.obj \
		  Service.obj

AOBJS		= Admin.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(AOBJS:.obj=.cpp)

HDIR		= $(includedir)\IceBox
SDIR		= $(slicedir)\IceBox

!include $(top_srcdir)\config\Make.rules.mak

!ifdef BUILD_UTILS

CPPFLAGS	= -I.. $(CPPFLAGS)

!else

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_BOX_API_EXPORTS 

!endif

SLICE2CPPFLAGS	= --checksum --ice --dll-export ICE_BOX_API --include-dir IceBox $(SLICE2CPPFLAGS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SERVER:.exe=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$(DLLNAME) $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

$(SERVER): $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) icebox$(LIBSUFFIX).lib

$(ADMIN): $(AOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) icebox$(LIBSUFFIX).lib

!ifdef BUILD_UTILS

clean::
	del /q IceBox.cpp $(HDIR)\IceBox.h
	del /q $(DLLNAME:.dll=.*)
	del /q $(SERVER:.exe=.*)
	del /q $(ADMIN:.exe=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(SERVER) $(install_bindir)
	copy $(ADMIN) $(install_bindir)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SERVER:.exe=.pdb) $(install_bindir)
	copy $(ADMIN:.exe=.pdb) $(install_bindir)

!endif

!else

install:: all

$(EVERYTHING)::
	$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
