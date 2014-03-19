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

TARGETS         = $(LIBNAME) $(DLLNAME) $(ADMIN)

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
		  Util.obj

AOBJS		= Admin.obj \
		  Grammar.obj \
		  Scanner.obj \
		  Parser.obj \
		  LLURecord.obj \
		  Election.obj \
		  SubscriberRecord.obj \
		  IceStormInternal.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(AOBJS:.obj=.cpp)

HDIR		= $(headerdir)\IceStorm
SDIR		= $(slicedir)\IceStorm

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude -DICE_STORM_SERVICE_API_EXPORTS $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
ICECPPFLAGS	= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	= --ice --include-dir IceStorm --dll-export ICE_STORM_SERVICE_API $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib icegrid$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib icedb$(LIBSUFFIX).lib
ALINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
!endif

RES_FILE        = IceStormService.res
ARES_FILE       = IceStormAdmin.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceStormService.res
	$(LINK) $(BASE):0x2C000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(ADMIN): $(AOBJS) IceStormAdmin.res 
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH) $(ARES_FILE)
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

clean::
	-del /q IceStormInternal.cpp IceStormInternal.h
	-del /q LLURecord.cpp LLURecord.h
	-del /q Election.cpp Election.h
	-del /q SubscriberRecord.cpp SubscriberRecord.h
	-del /q Instrumentation.cpp Instrumentation.h
	-del /q $(ADMIN:.exe=.*)
	-del /q IceStormAdmin.res IceStormDB.res IceStormService.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"
	copy $(ADMIN) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
        copy $(ADMIN:.exe=.pdb) "$(install_bindir)"
        copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

SUBDIRS = FreezeDB

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

!include .depend.mak
