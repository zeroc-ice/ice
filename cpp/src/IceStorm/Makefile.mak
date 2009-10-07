
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icestormdb$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icestormdb$(SOVERSION)$(LIBSUFFIX).dll

SVCLIBNAME_D	= $(top_srcdir)\lib\icestormserviced.lib
SVCDLLNAME_D	= $(top_srcdir)\bin\icestormservice$(SOVERSION)d.dll

SVCLIBNAME_R	= $(top_srcdir)\lib\icestormservice.lib
SVCDLLNAME_R	= $(top_srcdir)\bin\icestormservice$(SOVERSION).dll

SVCLIBNAME	= $(top_srcdir)\lib\icestormservice$(LIBSUFFIX).lib
SVCDLLNAME	= $(top_srcdir)\bin\icestormservice$(SOVERSION)$(LIBSUFFIX).dll

ADMIN		= $(top_srcdir)\bin\icestormadmin.exe

TARGETS         = $(LIBNAME) $(DLLNAME) $(SVCLIBNAME) $(SVCDLLNAME) $(ADMIN)

OBJS		= LLURecord.obj \
		  Election.obj \
		  SubscriberRecord.obj \
		  IceStormInternal.obj

SERVICE_OBJS	= NodeI.obj \
		  Observers.obj \
		  Instance.obj \
		  TraceLevels.obj \
		  Subscriber.obj \
		  TopicI.obj \
		  TopicManagerI.obj \
		  TransientTopicI.obj \
		  TransientTopicManagerI.obj \
		  Service.obj

AOBJS		= Admin.obj \
		  Grammar.obj \
		  Scanner.obj \
		  Parser.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(SERVICE_OBJS:.obj=.cpp) \
		  $(AOBJS:.obj=.cpp)

HDIR		= $(headerdir)\IceStorm
SDIR		= $(slicedir)\IceStorm

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
ICECPPFLAGS	= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	= --ice --include-dir IceStorm --dll-export ICE_STORM_LIB_API $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib
SVCLINKWITH 	= $(LIBS) icestormdb$(LIBSUFFIX).lib icestorm$(LIBSUFFIX).lib icegrid$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib icedb$(LIBSUFFIX).lib 
ALINKWITH 	= $(LIBS) icestormdb$(LIBSUFFIX).lib icestorm$(LIBSUFFIX).lib

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SVCDLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
!endif

RES_FILE        = IceStormDB.res
SRES_FILE       = IceStormService.res
ARES_FILE       = IceStormAdmin.res

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceStormDB.res
	$(LINK) $(LD_DLLFLAGS) $(DPDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(DRES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(SVCLIBNAME): $(SVCDLLNAME)

$(SVCDLLNAME): $(SERVICE_OBJS) IceStormService.res
	$(LINK) $(LD_DLLFLAGS) $(SPDBFLAGS) $(SERVICE_OBJS) $(PREOUT)$@ $(PRELIBS)$(SVCLINKWITH) $(SRES_FILE)
	move $(SVCDLLNAME:.dll=.lib) $(SVCLIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(SVCDLLNAME:.dll=.exp) del /q $(SVCDLLNAME:.dll=.exp)

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
	echo #include "IceUtil/Config.h" > Scanner.cpp
	type lex.yy.c >> Scanner.cpp
	del /q lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	del /q Grammar.h Grammar.cpp
	bison -dvt Grammar.y
	move Grammar.tab.c Grammar.cpp
	move Grammar.tab.h Grammar.h
	del /q Grammar.output

SUBDIRS = FreezeDB
!if "$(QT_HOME)" != ""
SUBDIRS = $(SUBDIRS) SqlDB
!endif

$(EVERYTHING)::
	@for %i in ( $(SUBDIRS) ) do \
	    @if exist %i \
	        @echo "making $@ in %i" && \
	        cmd /c "cd %i && $(MAKE) -nologo -f Makefile.mak $@" || exit 1

clean::
	-del /q IceStormInternal.cpp IceStormInternal.h
	-del /q LinkRecord.cpp LinkRecord.h
	-del /q LLURecord.cpp LLURecord.h
	-del /q Election.cpp Election.h
	-del /q SubscriberRecord.cpp SubscriberRecord.h
	-del /q $(SVCDLLNAME_R:.dll=.*) $(SVCDLLNAME_D:.dll=.*)
	-del /q $(SVCLIBNAME_R) $(SVCLIBNAME_D)
	-del /q $(ADMIN:.exe=.*)
	-del /q IceStormAdmin.res IceStormDB.res IceStormService.res

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(SVCLIBNAME) $(install_libdir)
	copy $(SVCDLLNAME) $(install_bindir)
	copy $(ADMIN) $(install_bindir)

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SVCDLLNAME:.dll=.pdb) $(install_bindir)
	copy $(ADMIN:.exe=.pdb) $(install_bindir)

!include .depend
