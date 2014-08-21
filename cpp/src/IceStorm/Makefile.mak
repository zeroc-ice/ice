# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icestormservice$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icestormservice$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

ADMIN		= $(top_srcdir)\bin\icestormadmin.exe
MIGRATE		= $(top_srcdir)\bin\icestormmigrate.exe

TARGETS         = $(LIBNAME) $(DLLNAME) $(ADMIN) $(MIGRATE)

OBJS		= NodeI.obj \
		  Observers.obj \
		  Instance.obj \
		  TraceLevels.obj \
		  Subscriber.obj \
		  TopicI.obj \
		  TopicManagerI.obj \
		  TransientTopicI.obj \
		  TransientTopicManagerI.obj \
		  Service.obj \
		  LLURecord.obj \
		  Election.obj \
		  SubscriberRecord.obj \
		  IceStormInternal.obj \
		  Instrumentation.obj \
		  InstrumentationI.obj \
		  Util.obj \
		  LLUMap.obj \
		  SubscriberMap.obj \
		  V32FormatDB.obj \
		  V31FormatDB.obj \
		  LinkRecord.obj \
		  V32Format.obj \
		  V31Format.obj

AOBJS		= Admin.obj \
		  Grammar.obj \
		  Scanner.obj \
		  Parser.obj \
		  LLURecord.obj \
		  Election.obj \
		  SubscriberRecord.obj \
		  IceStormInternal.obj

MOBJS		= Migrate.obj \
		  SubscriberMap.obj \
		  LLUMap.obj \
		  LinkRecord.obj \
		  V32FormatDB.obj \
		  V31FormatDB.obj \
		  V32Format.obj \
		  V31Format.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(AOBJS:.obj=.cpp) \
		  $(MOBJS:.obj=.cpp)

HDIR		= $(headerdir)\IceStorm
SDIR		= $(slicedir)\IceStorm

SLICE2FREEZECMD = $(SLICE2FREEZE) -I.. --ice --include-dir IceStorm $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. -DICE_STORM_SERVICE_API_EXPORTS $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
ICECPPFLAGS	= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	= --ice --include-dir IceStorm --dll-export ICE_STORM_SERVICE_API $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib icegrid$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib
ALINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib
MLINKWITH 	= freeze$(LIBSUFFIX).lib icestormservice$(LIBSUFFIX).lib icestorm$(LIBSUFFIX).lib $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
MPDBFLAGS       = /pdb:$(MIGRATE:.exe=.pdb)
!endif

RES_FILE        = IceStormService.res
ARES_FILE       = IceStormAdmin.res
MRES_FILE       = IceStormMigrate.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) $(RES_FILE)
	$(LINK) $(BASE):0x2C000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(ADMIN): $(AOBJS) $(ARES_FILE) 
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH) $(ARES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(MIGRATE): $(MOBJS) $(MRES_FILE)
        $(LINK) $(LD_EXEFLAGS) $(MPDBFLAGS) $(MOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(MLINKWITH) $(MRES_FILE)
        @if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
            $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

# Implicit rule to build the private IceStorm .ice files.
{..\IceStorm\}.ice{..\IceStorm\}.h:
	del /q $(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(*F).ice

Scanner.cpp: Scanner.l
	flex Scanner.l
	del /q $@
	echo #include "IceUtil/ScannerConfig.h" >> Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

LLUMap.h LLUMap.cpp: ..\IceStorm\LLURecord.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate LLUMap ..\IceStorm\LLURecord.ice

SubscriberMap.h SubscriberMap.cpp: ..\IceStorm\SubscriberRecord.ice $(slicedir)\Ice\Identity.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	del /q SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) \
	--dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap ..\IceStorm\SubscriberRecord.ice

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
	-del /q IceStormInternal.cpp IceStormInternal.h
	-del /q LLURecord.cpp LLURecord.h
	-del /q Election.cpp Election.h
	-del /q SubscriberRecord.cpp SubscriberRecord.h
	-del /q Instrumentation.cpp Instrumentation.h
	-del /q LLUMap.h LLUMap.cpp
	-del /q SubscriberMap.h SubscriberMap.cpp
	-del /q LinkRecord.cpp LinkRecord.h
	-del /q V32FormatDB.cpp V31FormatDB.cpp V32FormatDB.h V31FormatDB.h
	-del /q V32Format.cpp V32Format.h
	-del /q V31Format.cpp V31Format.h
	-del /q $(ADMIN:.exe=.*)
	-del /q $(MIGRATE:.exe=.*)
	-del /q IceStormAdmin.res IceStormDB.res IceStormService.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"
	copy $(ADMIN) "$(install_bindir)"
	copy $(MIGRATE) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
        copy $(ADMIN:.exe=.pdb) "$(install_bindir)"
        copy $(MIGRATE:.exe=.pdb) "$(install_bindir)"
        copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
