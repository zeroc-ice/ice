# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

!ifdef BUILD_UTILS

TARGETS         = $(ADMIN)

!else

TARGETS         = $(LIBNAME) $(DLLNAME) $(SVCLIBNAME) $(SVCDLLNAME)

!endif

OBJS		= IceStorm.obj \


SERVICE_OBJS	= IceStormInternal.obj \
		  LinkRecord.obj \
		  IceStorm.obj \
		  Instance.obj \
		  TraceLevels.obj \
		  BatchFlusher.obj \
		  SubscriberPool.obj \
		  Subscriber.obj \
		  TopicI.obj \
		  TopicManagerI.obj \
                  PersistentTopicMap.obj \
		  Event.obj \
		  Service.obj

AOBJS		= Admin.obj \
		  Grammar.obj \
		  Scanner.obj \
		  Parser.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(AOBJS:.obj=.cpp)

HDIR		= $(includedir)\IceStorm
SDIR		= $(slicedir)\IceStorm

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice --include-dir IceStorm $(SLICE2CPPFLAGS) -I..
LINKWITH 	= $(LIBS) freeze$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib
ALINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceStorm -I.. -I$(slicedir)

!ifndef BUILD_UTILS

CPPFLAGS	= $(CPPFLAGS) -DICE_STORM_API_EXPORTS

!endif

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SVCDLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(SVCLIBNAME): $(SVCDLLNAME)

$(SVCDLLNAME): $(SERVICE_OBJS)
	$(LINK) $(LD_DLLFLAGS) $(SPDBFLAGS) $(SERVICE_OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	move $(SVCDLLNAME:.dll=.lib) $(SVCLIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(SVCDLLNAME:.dll=.exp) del /q $(SVCDLLNAME:.dll=.exp)

$(ADMIN): $(AOBJS)
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

..\IceStorm\PersistentTopicMap.h PersistentTopicMap.cpp: ..\IceStorm\LinkRecord.ice $(slicedir)\Ice\Identity.ice $(SLICE2FREEZE)
	del /q PersistentTopicMap.h PersistentTopicMap.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::PersistentTopicMap,Ice::Identity,IceStorm::LinkRecordSeq \
	PersistentTopicMap ..\IceStorm\LinkRecord.ice

Event.cpp Event.h: Event.ice
	$(SLICE2CPP) --dll-export ICE_STORM_API $(SLICE2CPPFLAGS) Event.ice

IceStorm.cpp $(HDIR)\IceStorm.h: $(SDIR)\IceStorm.ice
	$(SLICE2CPP) --dll-export ICE_STORM_API $(SLICE2CPPFLAGS) $(SDIR)\IceStorm.ice
	move IceStorm.h $(HDIR)

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

!ifdef BUILD_UTILS

clean::
	del /q PersistentTopicMap.h PersistentTopicMap.cpp

clean::
	del /q IceStorm.cpp $(HDIR)\IceStorm.h
	del /q IceStormInternal.cpp IceStormInternal.h
	del /q LinkRecord.cpp LinkRecord.h
	del /q $(DLLNAME:.dll=.*)
	del /q $(SVCDLLNAME:.dll=.*)
	del /q $(ADMIN:.exe=.*)

clean::
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(SVCLIBNAME) $(install_libdir)
	copy $(SVCDLLNAME) $(install_bindir)
	copy $(ADMIN) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)
	copy $(SVCDLLNAME:.dll=.tds) $(install_bindir)
	copy $(ADMIN:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SVCDLLNAME:.dll=.pdb) $(install_bindir)
	copy $(ADMIN:.exe=.pdb) $(install_bindir)

!endif

!endif

!else

install:: all

$(EVERYTHING)::
	@$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
