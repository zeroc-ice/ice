# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

CPPFLAGS	= -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!else

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_BOX_API_EXPORTS 

!endif

SLICE2CPPFLAGS	= --checksum --ice --dll-export ICE_BOX_API --include-dir IceBox $(SLICE2CPPFLAGS)

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SERVER:.exe=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LIBS) icebox$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(ADMIN): $(AOBJS)
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LIBS) icebox$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

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

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)
	copy $(SERVER:.exe=.tds) $(install_bindir)
	copy $(ADMIN:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SERVER:.exe=.pdb) $(install_bindir)
	copy $(ADMIN:.exe=.pdb) $(install_bindir)

!endif

!endif

!else

install:: all

$(EVERYTHING)::
	@$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
