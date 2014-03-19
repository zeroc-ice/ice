# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icebox$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icebox$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

SERVER_D	= $(top_srcdir)\bin\iceboxd.exe
SERVER_R	= $(top_srcdir)\bin\icebox.exe

SERVER		= $(top_srcdir)\bin\icebox$(LIBSUFFIX).exe

ADMIN		= $(top_srcdir)\bin\iceboxadmin.exe

TARGETS         = $(LIBNAME) $(DLLNAME) $(SERVER) $(ADMIN)

OBJS		= IceBox.obj \
		  Exception.obj

SOBJS		= ServiceManagerI.obj \
		  Service.obj

AOBJS		= Admin.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(AOBJS:.obj=.cpp)

HDIR		= $(headerdir)\IceBox
SDIR		= $(slicedir)\IceBox

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

SLICE2CPPFLAGS	= --checksum --ice --dll-export ICE_BOX_API --include-dir IceBox $(SLICE2CPPFLAGS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SERVER:.exe=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
!endif

RES_FILE        = IceBox.res
SRES_FILE       = IceBoxExe.res
ARES_FILE       = IceBoxAdmin.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceBox.res
	$(LINK) $(BASE):0x26000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(SERVER): $(SOBJS) IceBoxExe.res
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icebox$(LIBSUFFIX).lib $(SRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(ADMIN): $(AOBJS) IceBoxAdmin.res
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icebox$(LIBSUFFIX).lib $(ARES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	-del /q IceBox.cpp $(HDIR)\IceBox.h
	-del /q $(SERVER_D:.exe=.*) $(SERVER_R:.exe=.*)
	-del /q $(ADMIN:.exe=.*)
	-del /q IceBox.res IceBoxAdmin.res IceBoxExe.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"
	copy $(SERVER) "$(install_bindir)"
	copy $(ADMIN) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"
	copy $(SERVER:.exe=.pdb) "$(install_bindir)"
	copy $(ADMIN:.exe=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
