# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icepatch2$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icepatch2$(SOVERSION)$(LIBSUFFIX).dll

SERVER		= $(top_srcdir)\bin\icepatch2server.exe
CLIENT		= $(top_srcdir)\bin\icepatch2client.exe
CALC		= $(top_srcdir)\bin\icepatch2calc.exe

!ifdef BUILD_UTILS

TARGETS         = $(SERVER) $(CLIENT) $(CALC)

!else

TARGETS         = $(LIBNAME) $(DLLNAME)

!endif

OBJS		= Util.obj \
		  ClientUtil.obj \
		  FileInfo.obj \
		  FileServer.obj \
		  OS.obj

SOBJS		= Server.obj \
		  FileServerI.obj

COBJS		= Client.obj

CALCOBJS	= Calc.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(CALCOBJS:.obj=.cpp)

HDIR		= $(includedir)\IcePatch2
SDIR		= $(slicedir)\IcePatch2

!include $(top_srcdir)\config\Make.rules.mak

!ifdef BUILD_UTILS

CPPFLAGS	= -I. -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!else

CPPFLAGS	= -I. -I.. $(CPPFLAGS) -DICE_PATCH2_API_EXPORTS -DWIN32_LEAN_AND_MEAN

!endif

SLICE2CPPFLAGS	= --ice --include-dir IcePatch2 --dll-export ICE_PATCH2_API $(SLICE2CPPFLAGS)

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SERVER:.exe=.pdb)
CPDBFLAGS       = /pdb:$(CLIENT:.exe=.pdb)
CAPDBFLAGS      = /pdb:$(CALC:.exe=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(BZIP2_LIBS) $(OPENSSL_LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CALC): $(CALCOBJS)
	$(LINK) $(LD_EXEFLAGS) $(CAPDBFLAGS) $(SETARGV) $(CALCOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#1 && del /q $@.manifest

!ifdef BUILD_UTILS

clean::
	del /q FileInfo.cpp $(HDIR)\FileInfo.h
	del /q FileServer.cpp $(HDIR)\FileServer.h
	del /q $(DLLNAME:.dll=.*)
	del /q $(SERVER:.exe=.*)
	del /q $(CLIENT:.exe=.*)
	del /q $(CALC:.exe=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(SERVER) $(install_bindir)
	copy $(CLIENT) $(install_bindir)
	copy $(CALC) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)
	copy $(SERVER:.exe=.tds) $(install_bindir)
	copy $(CLIENT:.exe=.tds) $(install_bindir)
	copy $(CALC:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SERVER:.exe=.pdb) $(install_bindir)
	copy $(CLIENT:.exe=.pdb) $(install_bindir)
	copy $(CALC:.exe=.pdb) $(install_bindir)

!endif

!endif

!else

install:: all

$(EVERYTHING)::
	@$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
