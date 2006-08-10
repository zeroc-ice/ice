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

OBJS		= IceStorm.o \

SOBJS		= TraceLevels.o \
		  Flusher.o \
		  Subscriber.o \
		  OnewaySubscriber.o \
		  OnewayBatchSubscriber.o \
		  LinkSubscriber.o \
		  SubscriberFactory.o \
		  TopicI.o \
		  TopicManagerI.o \
		  PersistentTopicMap.o \
		  LinkRecord.o \
		  IceStormInternal.o \
		  Service.o \
		  QueuedProxy.o \
		  OnewayProxy.o \
		  TwowayProxy.o \
		  LinkProxy.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  WeightedGraph.o \
		  Parser.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

HDIR		= $(includedir)\IceStorm
SDIR		= $(slicedir)\IceStorm

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. -Idummyinclude $(CPPFLAGS)
SLICE2CPPFLAGS	= --ice --include-dir IceStorm $(SLICE2CPPFLAGS)
LINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib freeze$(LIBSUFFIX).lib icebox$(LIBSUFFIX).lib
ALINKWITH 	= $(LIBS) icestorm$(LIBSUFFIX).lib icexml$(LIBSUFFIX).lib

!ifndef BUILD_UTILS

EXTRAFLAGS	= -DICE_STORM_SERVICE_API_EXPORTS

!endif

IceStorm.o: IceStorm.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -DICE_STORM_API_EXPORTS -o IceStorm.o IceStorm.cpp

.cpp.o:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(EXTRAFLAGS) -o $@ $<

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(OBJS), $(DLLNAME),, $(LIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

$(SVCLIBNAME): $(SVCDLLNAME)

$(SVCDLLNAME): $(SOBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(SOBJS), $(SVCDLLNAME),, $(LINKWITH)
	move $(SVCDLLNAME:.dll=.lib) $(SVCLIBNAME)

$(ADMIN): $(AOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(AOBJS), $@,, $(ALINKWITH)

PersistentTopicMap.h PersistentTopicMap.cpp: ../IceStorm/LinkRecord.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE)
	del /q PersistentTopicMap.h PersistentTopicMap.cpp
	$(SLICE2FREEZE) --ice --include-dir IceStorm -I.. -I$(slicedir) --dict \
	IceStorm::PersistentTopicMap,string,IceStorm::LinkRecordDict PersistentTopicMap \
	..\IceStorm\LinkRecord.ice

IceStorm.cpp $(HDIR)\IceStorm.h: $(SDIR)\IceStorm.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) --checksum --dll-export ICE_STORM_API $(SLICE2CPPFLAGS) $(SDIR)\IceStorm.ice
	move IceStorm.h $(HDIR)

IceStormInternal.cpp IceStormInternal.h: IceStormInternal.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) IceStormInternal.ice

LinkRecord.cpp LinkRecord.h: LinkRecord.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) -I.. $(SLICE2CPPFLAGS) LinkRecord.ice

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

clean::
	del /q Grammar.cpp Grammar.h
	del /q Scanner.cpp

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(SVCLIBNAME) $(install_libdir)
	copy $(SVCDLLNAME) $(install_bindir)
	copy $(ADMIN) $(install_bindir)

!else

install:: all

$(EVERYTHING)::
	$(MAKE) /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend
