
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

LIBNAME		= $(top_srcdir)\lib\icestormfreezedb$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icestormfreezedb$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

MIGRATE		= $(top_srcdir)\bin\icestormmigrate.exe

TARGETS         = $(LIBNAME) $(DLLNAME) $(MIGRATE)

OBJS   		= LLUMap.obj \
                  SubscriberMap.obj \
		  FreezeDB.obj

DB_OBJS		= FreezeTypes.obj

MOBJS		= Migrate.obj \
                  SubscriberMap.obj \
                  LLUMap.obj \
		  LinkRecord.obj \
		  V32FormatDB.obj \
		  V31FormatDB.obj \
		  V32Format.obj \
		  V31Format.obj

{$(top_srcdir)\src\IceDB\}.cpp.obj::
    $(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

SRCS		= $(OBJS:.obj=.cpp) \
		  $(MOBJS:.obj=.cpp) \
		  $(top_srcdir)\src\IceDB\FreezeTypes.cpp

HDIR		= $(headerdir)/IceStorm
SDIR		= $(slicedir)/IceStorm

SLICE2FREEZECMD = $(SLICE2FREEZE) -I..\.. --ice --include-dir IceStorm/FreezeDB $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I..\.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= -I..\.. --ice --include-dir IceStorm/FreezeDB $(SLICE2CPPFLAGS)

LINKWITH 	= icestormservice$(LIBSUFFIX).lib icestorm$(LIBSUFFIX).lib icedb$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib $(LIBS)
MLINKWITH 	= freeze$(LIBSUFFIX).lib icestormservice$(LIBSUFFIX).lib icestorm$(LIBSUFFIX).lib $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
MPDBFLAGS       = /pdb:$(MIGRATE:.exe=.pdb)
!endif

RES_FILE        = IceStormFreezeDB.res
MRES_FILE       = IceStormMigrate.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) $(DB_OBJS) IceStormFreezeDB.res
	$(LINK) /base:0x2D000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(DB_OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) \
		$(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(MIGRATE): $(MOBJS) IceStormMigrate.res
	$(LINK) $(LD_EXEFLAGS) $(MPDBFLAGS) $(MOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(MLINKWITH) $(MRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

LLUMap.h LLUMap.cpp: ..\..\IceStorm\LLURecord.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate LLUMap ..\..\IceStorm\LLURecord.ice

SubscriberMap.h SubscriberMap.cpp: ..\..\IceStorm\SubscriberRecord.ice $(slicedir)\Ice\Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) \
	--dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap ..\..\IceStorm\SubscriberRecord.ice

# Needed for migration.
V32FormatDB.h V32FormatDB.cpp: V32Format.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q V32FormatDB.h V32FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V32Format,Ice::Identity,IceStorm::LinkRecordSeq \
	V32FormatDB V32Format.ice

V31FormatDB.h V31FormatDB.cpp: V31Format.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q V31FormatDB.h V31FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V31Format,string,IceStorm::LinkRecordDict \
	V31FormatDB V31Format.ice

clean::
	-del /q LLUMap.h LLUMap.cpp
	-del /q SubscriberMap.h SubscriberMap.cpp
	-del /q LinkRecord.cpp LinkRecord.h
	-del /q V32FormatDB.cpp V31FormatDB.cpp V32FormatDB.h V31FormatDB.h
	-del /q V32Format.cpp V32Format.h
	-del /q V31Format.cpp V31Format.h
	-del /q $(MIGRATE:.exe=.*)
	-del /q IceStormFreezeDB.res IceStormMigrate.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"
	copy $(MIGRATE) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"
	copy $(MIGRATE:.exe=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
