# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

SOBJS		= TraceLevels.obj \
		  Flusher.obj \
		  Subscriber.obj \
		  OnewaySubscriber.obj \
		  OnewayBatchSubscriber.obj \
		  LinkSubscriber.obj \
		  SubscriberFactory.obj \
		  TopicI.obj \
		  TopicManagerI.obj \
		  PersistentTopicMap.obj \
		  LinkRecord.obj \
		  IceStormInternal.obj \
		  Service.obj \
		  QueuedProxy.obj \
		  OnewayProxy.obj \
		  TwowayProxy.obj \
		  LinkProxy.obj

AOBJS		= Admin.obj \
		  Grammar.obj \
		  Scanner.obj \
		  WeightedGraph.obj \
		  Parser.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(AOBJS:.obj=.cpp)

HDIR		= $(includedir)\IceStorm
SDIR		= $(slicedir)\IceStorm

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude $(CPPFLAGS)
SLICE2CPPFLAGS	= --ice --include-dir IceStorm $(SLICE2CPPFLAGS) -I..
LINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib
ALINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib

!ifndef BUILD_UTILS

EXTRAFLAGS	= -DICE_STORM_SERVICE_API_EXPORTS

!endif

IceStorm.obj: IceStorm.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -DICE_STORM_API_EXPORTS IceStorm.cpp

.cpp.obj::
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(EXTRAFLAGS) $<


!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
SPDBFLAGS       = /pdb:$(SVCDLLNAME:.dll=.pdb)
APDBFLAGS       = /pdb:$(ADMIN:.exe=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$(DLLNAME) $(PRELIBS)$(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

$(SVCLIBNAME): $(SVCDLLNAME)

$(SVCDLLNAME): $(SOBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(SPDBFLAGS) $(SOBJS) $(PREOUT)$(SVCDLLNAME) $(PRELIBS)$(LINKWITH)
	move $(SVCDLLNAME:.dll=.lib) $(SVCLIBNAME)

$(ADMIN): $(AOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(APDBFLAGS) $(AOBJS) $(PREOUT)$@ $(PRELIBS)$(ALINKWITH)

PersistentTopicMap.h PersistentTopicMap.cpp: ../IceStorm/LinkRecord.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE)
	del /q PersistentTopicMap.h PersistentTopicMap.cpp
	$(SLICE2FREEZE) --ice --include-dir IceStorm -I.. -I$(slicedir) --dict \
	IceStorm::PersistentTopicMap,string,IceStorm::LinkRecordDict PersistentTopicMap \
	..\IceStorm\LinkRecord.ice

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

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SVCDLLNAME:.dll=.pdb) $(install_bindir)
	copy $(ADMIN:.exe=.pdb) $(install_bindir)

!endif

!else

install:: all

$(EVERYTHING)::
	$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
