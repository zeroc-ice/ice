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

NAME		= $(top_srcdir)/lib/libYellow.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

SNAME		= $(top_srcdir)/lib/libYellowService.$(LIBEXT)
VERSIONED_SNAME	= $(SNAME).$(VERSION)

ADMIN		= $(top_srcdir)/bin/yellowadmin

TARGETS		= $(NAME) $(VERSIONED_NAME) $(SNAME) $(VERSIONED_SNAME) $(SERVER) $(ADMIN)

OBJS		= Yellow.o

SERVICE_OBJS	= Yellow.o \
		  StringObjectProxySeqDict.o \
		  AdminI.o \
		  QueryI.o \
		  Service.o

AOBJS		= Admin.o \
		  Grammar.o \
		  Scanner.o \
		  Parser.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp) \
		  $(AOBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/Yellow.ice

HDIR		= $(includedir)/Yellow
SDIR		= $(slicedir)/Yellow

SLICECMD	= $(SLICE2CPP) --ice --include-dir Yellow $(ICECPPFLAGS)
SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir Yellow $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. -I$(DB_HOME)/include $(CPPFLAGS)
LDFLAGS		:= $(LDFLAGS) -L$(DB_HOME)/lib

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(VERSIONED_SNAME): $(SERVICE_OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(SERVICE_OBJS)

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lYellow $(LIBS) -lreadline -lcurses

$(HDIR)/Yellow.h Yellow.cpp: $(SDIR)/Yellow.ice $(SLICE2CPP)
	rm -f $(HDIR)/Yellow.h Yellow.cpp
	$(SLICECMD) --dll-export ICE_YELLOW_API $(SDIR)/Yellow.ice
	mv Yellow.h $(HDIR)

clean::
	rm -f $(HDIR)/Yellow.h Yellow.cpp

StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp
	$(SLICE2FREEZECMD) --dict Yellow::StringObjectProxySeqDict,string,Ice::ObjectProxySeq \
	StringObjectProxySeqDict $(slicedir)/Ice/BuiltinSequences.ice

clean::
	rm -f StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp

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
