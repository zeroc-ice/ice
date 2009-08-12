# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

TARGETS         = $(LIBNAME) $(DLLNAME) $(SERVER) $(CLIENT) $(CALC)

OBJS		= Util.obj \
		  ClientUtil.obj \
		  FileInfo.obj \
		  FileServer.obj

SOBJS		= Server.obj \
		  FileServerI.obj

COBJS		= Client.obj

CALCOBJS	= Calc.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(CALCOBJS:.obj=.cpp)

HDIR		= $(headerdir)\IcePatch2
SDIR		= $(slicedir)\IcePatch2

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

SLICE2CPPFLAGS	= --ice --include-dir IcePatch2 --dll-export ICE_PATCH2_API $(SLICE2CPPFLAGS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SERVER:.exe=.pdb)
CPDBFLAGS       = /pdb:$(CLIENT:.exe=.pdb)
CAPDBFLAGS      = /pdb:$(CALC:.exe=.pdb)
!endif

!if "$(BCPLUSPLUS)" == "yes"
RES_FILE        = ,, IcePatch2.res
SRES_FILE       = ,, IcePatch2Server.res
CRES_FILE       = ,, IcePatch2Client.res
CARES_FILE      = ,, IcePatch2Calc.res
!else
RES_FILE        = IcePatch2.res
SRES_FILE       = IcePatch2Server.res
CRES_FILE       = IcePatch2Client.res
CARES_FILE      = IcePatch2Calc.res
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IcePatch2.res
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(BZIP2_LIBS) \
		$(OPENSSL_LIBS) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(SERVER): $(SOBJS) IcePatch2Server.res
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib $(SRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CLIENT): $(COBJS) IcePatch2Client.res
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib $(CRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#1 && del /q $@.manifest

$(CALC): $(CALCOBJS) IcePatch2Calc.res
	$(LINK) $(LD_EXEFLAGS) $(CAPDBFLAGS) $(SETARGV) $(CALCOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib $(CARES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	-del /q FileInfo.cpp $(HDIR)\FileInfo.h
	-del /q FileServer.cpp $(HDIR)\FileServer.h
	-del /q $(SERVER:.exe=.*)
	-del /q $(CLIENT:.exe=.*)
	-del /q $(CALC:.exe=.*)
	-del /q IcePatch.res IcePatch2Server.res IcePatch2Client.res IcePatch2Calc.res

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(SERVER) $(install_bindir)
	copy $(CLIENT) $(install_bindir)
	copy $(CALC) $(install_bindir)


!if "$(BCPLUSPLUS)" == "yes" && "$(OPTIMIZE)" != "yes"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)
	copy $(SERVER:.exe=.tds) $(install_bindir)
	copy $(CLIENT:.exe=.tds) $(install_bindir)
	copy $(CALC:.exe=.tds) $(install_bindir)

!elseif "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SERVER:.exe=.pdb) $(install_bindir)
	copy $(CLIENT:.exe=.pdb) $(install_bindir)
	copy $(CALC:.exe=.pdb) $(install_bindir)

!endif

!include .depend
