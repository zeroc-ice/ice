#**********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
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
		  TopicI.o \
		  TopicManagerI.o \
		  StringBoolDict.o \
		  Server.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  Parser.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

HDIR		= $(includedir)/IceStorm
SDIR		= $(slicedir)/IceStorm
SLICECMD	= $(SLICE2CPP) --include-dir IceStorm --dll-export ICE_STORM_API -I$(slicedir)
SLICE2FREEZECMD = $(SLICE2FREEZE) --include-dir IceStorm

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)
LDFLAGS		:= $(LDFLAGS) -L$(DB)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared \
	-o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOBJS) -lIceStorm -lFreeze -ldb $(LIBS)

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(AOBJS) -lIceStorm $(LIBS) -lreadline -lcurses

$(HDIR)/IceStorm.h IceStorm.cpp: $(SDIR)/IceStorm.ice $(SLICE2CPP)
	rm -f $(HDIR)/IceStorm.h IceStorm.cpp
	$(SLICECMD) $(SDIR)/IceStorm.ice
	mv IceStorm.h $(HDIR)

StringBoolDict.h StringBoolDict.cpp: $(SLICE2FREEZE)
	rm -f StringBoolDict.h StringBoolDict.cpp
	$(SLICE2FREEZECMD) --dict IceStorm::StringBoolDict,string,bool StringBoolDict

Grammar.cpp Grammar.h: Grammar.y
	bison -dvt Grammar.y
	rm -f Grammar.cpp ; mv Grammar.tab.c Grammar.cpp
	rm -f Grammar.h ; mv Grammar.tab.h Grammar.h

Scanner.cpp: Scanner.l Grammar.h
	flex Scanner.l
	rm -f Scanner.cpp ; mv lex.yy.c Scanner.cpp

clean::
	-rm -f Grammar.cpp Grammar.h Grammar.output
	-rm -f Scanner.cpp lex.yy.c

clean::
	rm -f $(HDIR)/Clock.h Clock.cpp

include .depend
