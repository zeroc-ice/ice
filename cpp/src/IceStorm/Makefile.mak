# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icestormservice$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icestormservice$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

ADMIN		= $(top_srcdir)\bin\icestormadmin.exe
DB		= $(top_srcdir)\bin\icestormdb.exe
MIGRATE		= $(top_srcdir)\bin\icestormmigrate.exe

TARGETS		= $(LIBNAME) $(DLLNAME) $(ADMIN) $(MIGRATE) $(DB)

CSLICE_OBJS     = .\Election.obj \
		  .\IceStormInternal.obj \
		  .\Instrumentation.obj \
		  .\LinkRecord.obj \
		  .\LLURecord.obj \
		  .\SubscriberRecord.obj \
		  .\V31Format.obj \
		  .\V32Format.obj

DSLICE_OBJS     = .\DBTypes.obj

BISON_FLEX_OBJS = .\Grammar.obj \
                  .\Scanner.obj

LIB_OBJS	= .\Instance.obj \
		  .\InstrumentationI.obj \
		  .\LLUMap.obj \
		  .\NodeI.obj \
		  .\Observers.obj \
		  .\Service.obj \
		  .\Subscriber.obj \
		  .\SubscriberMap.obj \
		  .\TopicI.obj \
		  .\TopicManagerI.obj \
		  .\TraceLevels.obj \
		  .\TransientTopicI.obj \
		  .\TransientTopicManagerI.obj \
		  .\Util.obj \
		  .\V31FormatDB.obj \
		  .\V32FormatDB.obj \
                  $(CSLICE_OBJS)

AOBJS		= .\Admin.obj \
		  .\Parser.obj \
                  $(CSLICE_OBJS) \
                  $(BISON_FLEX_OBJS)

MOBJS		= .\LLUMap.obj \
                  .\Migrate.obj \
                  .\SubscriberMap.obj \
                  .\V31FormatDB.obj \
		  .\V32FormatDB.obj \
                  $(CSLICE_OBJS)

DOBJS		= .\IceStormDB.obj \
                  .\LLUMap.obj \
                  .\SubscriberMap.obj \
                  $(DSLICE_OBJS)

OBJS		= $(LIB_OBJS) \
		  $(AOBJS) \
		  $(MOBJS) \
		  $(DOBJS)

SLICE_OBJS	= $(CSLICE_OBJS) \
		  $(DSLICE_OBJS)

HDIR		= $(headerdir)\IceStorm
SDIR		= $(slicedir)\IceStorm

SLICE2FREEZECMD = $(SLICE2FREEZE) -I.. --ice --include-dir IceStorm $(ICECPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
ICECPPFLAGS	= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	= --ice --include-dir IceStorm -I. $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS)
ALINKWITH 	= $(LIBS) 
MLINKWITH 	= $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
DPDBFLAGS       = /pdb:$(DB:.exe=.pdb)
MPDBFLAGS       = /pdb:$(MIGRATE:.exe=.pdb)
!endif

RES_FILE        = IceStormService.res
ARES_FILE       = IceStormAdmin.res
DRES_FILE       = IceStormDB.res
MRES_FILE       = IceStormMigrate.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LIB_OBJS) $(RES_FILE)
	$(LINK) $(BASE):0x2C000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(LIB_OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(ADMIN): $(AOBJS) $(ARES_FILE)
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH) $(ARES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(DB): $(DOBJS) $(DRES_FILE)
	$(LINK) $(LD_EXEFLAGS) $(DPDBFLAGS) $(DOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(DRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(MIGRATE): $(MOBJS) $(MRES_FILE)
	$(LINK) $(LD_EXEFLAGS) $(MPDBFLAGS) $(MOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(MLINKWITH) $(MRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

# Implicit rule to build the private IceStorm .ice files.
{..\IceStorm\}.ice{..\IceStorm\}.h:
	del /q $(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SLICE2CPPFLAGS) $(*F).ice

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
	-del /q DBTypes.cpp DBTypes.h
	-del /q LLUMap.h LLUMap.cpp
	-del /q IceStormInternal.cpp IceStormInternal.h
	-del /q Instrumentation.cpp Instrumentation.h
	-del /q LinkRecord.cpp LinkRecord.h
	-del /q LLUMap.h LLUMap.cpp
	-del /q LLURecord.cpp LLURecord.h
	-del /q SubscriberMap.h SubscriberMap.cpp
	-del /q SubscriberRecord.cpp SubscriberRecord.h
	-del /q V31Format.cpp V31Format.h
	-del /q V32Format.cpp V32Format.h
	-del /q V32FormatDB.cpp V31FormatDB.cpp V32FormatDB.h V31FormatDB.h
	-del /q $(ADMIN:.exe=.*)
	-del /q $(MIGRATE:.exe=.*)
	-del /q $(DB:.exe=.*)
	-del /q IceStormAdmin.res IceStormMigrate.res IceStormService.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"
	copy $(ADMIN) "$(install_bindir)"
	copy $(DB) "$(install_bindir)"
	copy $(MIGRATE) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
        copy $(ADMIN:.exe=.pdb) "$(install_bindir)"
        copy $(DB:.exe=.pdb) "$(install_bindir)"
        copy $(MIGRATE:.exe=.pdb) "$(install_bindir)"
        copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif
