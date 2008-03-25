# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icestorm$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icestorm$(SOVERSION)$(LIBSUFFIX).dll

SVCLIBNAME	= $(top_srcdir)\lib\icestormservice$(LIBSUFFIX).lib
SVCDLLNAME	= $(top_srcdir)\bin\icestormservice$(SOVERSION)$(LIBSUFFIX).dll

ADMIN		= $(top_srcdir)\bin\icestormadmin.exe
MIGRATE		= $(top_srcdir)\bin\icestormmigrate.exe

!ifdef BUILD_UTILS

TARGETS         = $(SVCLIBNAME) $(SVCDLLNAME) $(ADMIN) $(MIGRATE)

!else

TARGETS         = $(LIBNAME) $(DLLNAME)

!endif

OBJS		= IceStorm.obj

SERVICE_OBJS	= NodeI.obj \
		  Observers.obj \
		  LLUMap.obj \
		  Election.obj \
		  Instance.obj \
		  TraceLevels.obj \
		  Subscriber.obj \
		  TopicI.obj \
		  TopicManagerI.obj \
		  TransientTopicI.obj \
		  TransientTopicManagerI.obj \
                  SubscriberMap.obj \
		  SubscriberRecord.obj \
		  IceStormInternal.obj \
		  Service.obj

AOBJS		= Admin.obj \
		  Grammar.obj \
		  Scanner.obj \
		  Parser.obj \
		  Election.obj \
		  SubscriberRecord.obj \
		  IceStormInternal.obj

MOBJS		= Migrate.obj \
		  SubscriberRecord.obj \
                  SubscriberMap.obj \
                  LLUMap.obj \
		  LinkRecord.obj \
		  IceStormInternal.obj \
		  Election.obj \
                  V32FormatDB.obj \
                  V31FormatDB.obj \
                  V32Format.obj \
                  V31Format.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(AOBJS:.obj=.cpp) \
		  $(MOBJS:.obj=.cpp)

HDIR		= $(includedir)\IceStorm
SDIR		= $(slicedir)\IceStorm

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice --include-dir IceStorm $(SLICE2CPPFLAGS) -I..
LINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib icegrid$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib
ALINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib
MLINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceStorm -I.. -I$(slicedir)

!ifdef BUILD_UTILS

CPPFLAGS	= $(CPPFLAGS) -DICE_STORM_API_EXPORTS

!else

CPPFLAGS	= $(CPPFLAGS) -DICE_STORM_LIB_API_EXPORTS

!endif

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SVCDLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
MPDBFLAGS       = /pdb:$(MIGRATE:.exe=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceStorm.res
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) IceStorm.res $(PREOUT)$@ $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(SVCLIBNAME): $(SVCDLLNAME)

$(SVCDLLNAME): $(SERVICE_OBJS) IceStormService.res
	$(LINK) $(LD_DLLFLAGS) $(SPDBFLAGS) $(SERVICE_OBJS) IceStormService.res $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	move $(SVCDLLNAME:.dll=.lib) $(SVCLIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(SVCDLLNAME:.dll=.exp) del /q $(SVCDLLNAME:.dll=.exp)

$(ADMIN): $(AOBJS) IceStormAdmin.res 
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) IceStormAdmin.res $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(MIGRATE): $(MOBJS) IceStormMigrate.res
	$(LINK) $(LD_EXEFLAGS) $(MPDBFLAGS) $(MOBJS) IceStormMigrate.res $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(MLINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

LLUMap.h LLUMap.cpp: $(SLICE2FREEZE) ..\IceStorm\Election.ice
	del /q LLUMap.h LLUMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::LLUMap,string,IceStormElection::LogUpdate LLUMap ..\IceStorm\Election.ice

..\IceStorm\SubscriberMap.h SubscriberMap.cpp: ..\IceStorm\SubscriberRecord.ice $(slicedir)\Ice\Identity.ice $(SLICE2FREEZE)
	del /q SubscriberMap.h SubscriberMap.cpp
	$(SLICE2FREEZECMD) \
	--dict IceStorm::SubscriberMap,IceStorm::SubscriberRecordKey,IceStorm::SubscriberRecord,sort \
	SubscriberMap ..\IceStorm\SubscriberRecord.ice

# Needed for migration.
V32FormatDB.h V32FormatDB.cpp: ..\IceStorm\V32Format.ice $(SLICE2FREEZE)
	del /q V32FormatDB.h V32FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V32Format,Ice::Identity,IceStorm::LinkRecordSeq \
	V32FormatDB ..\IceStorm\V32Format.ice

V31FormatDB.h V31FormatDB.cpp: ..\IceStorm\V31Format.ice $(SLICE2FREEZE)
	del /q V31FormatDB.h V31FormatDB.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::V31Format,string,IceStorm::LinkRecordDict \
	V31FormatDB ..\IceStorm\V31Format.ice

V32Migrate.cpp ..\IceStorm\V32Migrate.h: ..\IceStorm\V32Migrate.ice
	$(SLICE2CPP) $(SLICE2CPPFLAGS) ..\IceStorm\V32Migrate.ice

V31Migrate.cpp ..\IceStorm\V31Migrate.h: ..\IceStorm\V31Migrate.ice
	$(SLICE2CPP) $(SLICE2CPPFLAGS) ..\IceStorm\V31Migrate.ice

LinkRecord.cpp ..\IceStorm\LinkRecord.h: ..\IceStorm\LinkRecord.ice
	$(SLICE2CPP) $(SLICE2CPPFLAGS) ..\IceStorm\LinkRecord.ice

SubscriberRecord.cpp ..\IceStorm\SubscriberRecord.h: ..\IceStorm\SubscriberRecord.ice
	$(SLICE2CPP) $(SLICE2CPPFLAGS) ..\IceStorm\SubscriberRecord.ice

IceStorm.cpp $(HDIR)\IceStorm.h: $(SDIR)\IceStorm.ice
	$(SLICE2CPP) --dll-export ICE_STORM_LIB_API $(SLICE2CPPFLAGS) $(SDIR)\IceStorm.ice
	move IceStorm.h $(HDIR)

Election.cpp ..\IceStorm\Election.h: ..\IceStorm\Election.ice
	$(SLICE2CPP) $(SLICE2CPPFLAGS) ..\IceStorm\Election.ice

Scanner.cpp : Scanner.l
	flex Scanner.l
	del /q $@
	echo #include "IceUtil/Config.h" > Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

IceStorm.res: IceStorm.rc
	rc.exe $(RCFLAGS) IceStorm.rc

IceStormService.res: IceStormService.rc
	rc.exe $(RCFLAGS) IceStormService.rc

IceStormAdmin.res: IceStormAdmin.rc
	rc.exe $(RCFLAGS) IceStormAdmin.rc

IceStormMigrate.res: IceStormMigrate.rc
	rc.exe $(RCFLAGS) IceStormMigrate.rc

!ifdef BUILD_UTILS

clean::
	del /q LLUMap.h LLUMap.cpp
	del /q SubscriberMap.h SubscriberMap.cpp
	del /q V32FormatDB.cpp V31FormatDB.cpp V31FormatDB.h V31FormatDB.h

clean::
	del /q IceStorm.cpp $(HDIR)\IceStorm.h
	del /q IceStormInternal.cpp IceStormInternal.h
	del /q V32Migrate.cpp V32Migrate.h
	del /q V31Migrate.cpp V31Migrate.h
	del /q LinkRecord.cpp LinkRecord.h
	del /q Election.cpp Election.h
	del /q SubscriberRecord.cpp SubscriberRecord.h
	del /q $(DLLNAME:.dll=.*)
	del /q $(SVCDLLNAME:.dll=.*)
	del /q $(ADMIN:.exe=.*)
	del /q $(MIGRATE:.exe=.*)
	del /q IceStormAdmin.res IceStormMigrate.res IceStorm.res IceStormService.res

clean::
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(SVCLIBNAME) $(install_libdir)
	copy $(SVCDLLNAME) $(install_bindir)
	copy $(ADMIN) $(install_bindir)
	copy $(MIGRATE) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)
	copy $(SVCDLLNAME:.dll=.tds) $(install_bindir)
	copy $(ADMIN:.exe=.tds) $(install_bindir)
	copy $(MIGRATE:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SVCDLLNAME:.dll=.pdb) $(install_bindir)
	copy $(ADMIN:.exe=.pdb) $(install_bindir)
	copy $(MIGRATE:.exe=.pdb) $(install_bindir)

!endif

!endif

!else

install:: all

$(EVERYTHING)::
	$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
