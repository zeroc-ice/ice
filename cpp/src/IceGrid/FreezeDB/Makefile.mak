# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

LIBNAME		= $(top_srcdir)\lib\icegridfreezedb$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icegridfreezedb$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS         = $(LIBNAME) $(DLLNAME)

OBJS   		= StringApplicationInfoDict.obj \
		  IdentityObjectInfoDict.obj \
		  StringAdapterInfoDict.obj \
		  SerialsDict.obj \
		  FreezeDB.obj

DB_OBJS		= FreezeTypes.obj

{$(top_srcdir)\src\IceDB\}.cpp.obj::
    $(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

SRCS		= $(OBJS:.obj=.cpp) \
		  $(top_srcdir)\src\IceDB\FreezeTypes.cpp

HDIR		= $(headerdir)/IceGrid
SDIR		= $(slicedir)/IceGrid

SLICE2FREEZECMD = $(SLICE2FREEZE) -I..\.. --ice --include-dir IceGrid/FreezeDB $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I..\.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= -I..\.. --ice --include-dir IceGrid/FreezeDB $(SLICE2CPPFLAGS)

LINKWITH 	= icegrid$(LIBSUFFIX).lib glacier2$(LIBSUFFIX).lib icedb$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib $(LIBS)
MLINKWITH 	= freeze$(LIBSUFFIX).lib icegrid$(LIBSUFFIX).lib icegriddb$(LIBSUFFIX).lib $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = IceGridFreezeDB.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) $(DB_OBJS) IceGridFreezeDB.res
	$(LINK) /base:0x2E000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(DB_OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) \
		$(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

StringApplicationInfoDict.h StringApplicationInfoDict.cpp: $(SDIR)\Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringApplicationInfoDict,string,IceGrid::ApplicationInfo \
	StringApplicationInfoDict $(SDIR)\Admin.ice

IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp: $(slicedir)\Ice\Identity.ice $(SDIR)\Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::IdentityObjectInfoDict,Ice::Identity,IceGrid::ObjectInfo \
	--dict-index IceGrid::IdentityObjectInfoDict,type \
	IdentityObjectInfoDict $(slicedir)\Ice\Identity.ice $(SDIR)\Admin.ice

StringAdapterInfoDict.h StringAdapterInfoDict.cpp: $(SDIR)\Admin.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::StringAdapterInfoDict,string,IceGrid::AdapterInfo \
	--dict-index IceGrid::StringAdapterInfoDict,replicaGroupId StringAdapterInfoDict $(SDIR)\Admin.ice

SerialsDict.h SerialsDict.cpp: $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q SerialsDict.h SerialsDict.cpp
	$(SLICE2FREEZECMD) --dict IceGrid::SerialsDict,string,long SerialsDict

clean::
	-del /q StringApplicationInfoDict.h StringApplicationInfoDict.cpp
	-del /q StringAdapterInfoDict.h StringAdapterInfoDict.cpp
	-del /q IdentityObjectInfoDict.h IdentityObjectInfoDict.cpp
	-del /q SerialsDict.h SerialsDict.cpp
	-del /q IceGridFreezeDB.res IceGridMigrate.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
