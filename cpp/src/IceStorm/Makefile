#**********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../..

NAME		= $(top_srcdir)/lib/libIceStorm.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

SNAME		= $(top_srcdir)/lib/libIceStormService.$(LIBEXT)
VERSIONED_SNAME	= $(SNAME).$(VERSION)

ADMIN		= $(top_srcdir)/bin/icestormadmin

TARGETS		= $(NAME) $(VERSIONED_NAME) $(SNAME) $(VERSIONED_SNAME) $(ADMIN)

OBJS		= IceStorm.o

SERVICE_OBJS	= IceStorm.o \
		  TraceLevels.o \
		  Flusher.o \
		  Subscriber.o \
		  OnewaySubscriber.o \
		  OnewayBatchSubscriber.o \
		  LinkSubscriber.o \
		  SubscriberFactory.o \
		  TopicI.o \
		  TopicManagerI.o \
		  StringBoolDict.o \
		  IdentityLinkDict.o \
		  LinkDB.o \
		  IceStormInternal.o \
		  Service.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  WeightedGraph.o \
		  Parser.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp) \
		  $(SERVER_OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IceStorm.ice \
		  IceStormInternal.ice \
		  LinkDB.ice

HDIR		= $(includedir)/IceStorm
SDIR		= $(slicedir)/IceStorm

SLICECMD	= $(SLICE2CPP) --ice --include-dir IceStorm $(ICECPPFLAGS)
SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir IceStorm $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. -I$(DB_HOME)/include $(XERCESC_FLAGS) $(CPPFLAGS)
ICECPPFLAGS	:= -I.. $(ICECPPFLAGS)
LDFLAGS		:= $(LDFLAGS) -L$(DB_HOME)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(VERSIONED_SNAME): $(SERVICE_OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(SERVICE_OBJS)

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(LIBS) $(XERCESC_LIBS) -lreadline -lcurses

$(HDIR)/IceStorm.h IceStorm.cpp: $(SDIR)/IceStorm.ice $(SLICE2CPP)
	rm -f $(HDIR)/IceStorm.h IceStorm.cpp
	$(SLICECMD) --dll-export ICE_STORM_API $(SDIR)/IceStorm.ice
	mv IceStorm.h $(HDIR)

clean::
	rm -f $(HDIR)/IceStorm.h IceStorm.cpp

IceStormInternal.h IceStormInternal.cpp: IceStormInternal.ice $(SLICE2CPP)
	rm -f IceStormInternal.h IceStormInternal.cpp
	$(SLICECMD) IceStormInternal.ice

clean::
	rm -f IceStormInternal.h IceStormInternal.cpp

StringBoolDict.h StringBoolDict.cpp: $(SLICE2FREEZE)
	rm -f StringBoolDict.h StringBoolDict.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::StringBoolDict,string,bool StringBoolDict

clean::
	rm -f StringBoolDict.h StringBoolDict.cpp

LinkDB.h LinkDB.cpp: LinkDB.ice $(SLICE2CPP)
	rm -f LinkDB.h LinkDB.cpp
	$(SLICECMD) LinkDB.ice

clean::
	rm -f LinkDB.h LinkDB.cpp

IdentityLinkDict.h IdentityLinkDict.cpp: ../IceStorm/LinkDB.ice $(slicedir)/Ice/Identity.ice $(SLICE2FREEZE)
	rm -f IdentityLinkDict.h IdentityLinkDict.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::IdentityLinkDict,Ice::Identity,IceStorm::LinkDB IdentityLinkDict \
	../IceStorm/LinkDB.ice $(slicedir)/Ice/Identity.ice

clean::
	rm -f IdentityLinkDict.h IdentityLinkDict.cpp

Grammar.cpp Grammar.h: Grammar.y
	bison -dvt Grammar.y
	rm -f Grammar.cpp ; mv Grammar.tab.c Grammar.cpp
	rm -f Grammar.h ; mv Grammar.tab.h Grammar.h

Scanner.cpp: Scanner.l Grammar.h
	flex Scanner.l
	rm -f Scanner.cpp
	echo '#include <IceUtil/Config.h>' > Scanner.cpp
	cat lex.yy.c >> Scanner.cpp

clean::
	-rm -f Grammar.cpp Grammar.h Grammar.output
	-rm -f Scanner.cpp lex.yy.c

include .depend
