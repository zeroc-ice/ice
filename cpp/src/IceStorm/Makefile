#**********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

BASE		= libIceStorm.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

SERVER		= $(top_srcdir)/bin/icestorm
ADMIN		= $(top_srcdir)/bin/icestormadmin

TARGETS		= $(NAME) $(VERSIONED_NAME) $(SERVER) $(ADMIN)

OBJS		= IceStorm.o

SOBJS		= TraceLevels.o \
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
		  Server.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  WeightedGraph.o \
		  Parser.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

HDIR		= $(includedir)/IceStorm
SDIR		= $(slicedir)/IceStorm
SLICECMD	= $(SLICE2CPP) --include-dir IceStorm -I$(slicedir) -I..
SLICE2FREEZECMD = $(SLICE2FREEZE) --include-dir IceStorm -I$(slicedir) -I..

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. -I$(DB_HOME)/include $(XERCESC_FLAGS) $(CPPFLAGS)
LDFLAGS		:= $(LDFLAGS) -L$(DB_HOME)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lIceStorm -lFreeze $(LIBS) $(XERCESC_LIBS) -ldb

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(LIBS) $(XERCESC_LIBS) -lreadline -lcurses

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
