#**********************************************************************
#
# Copyright (c) 2002
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

SLICE2FREEZECMD = $(SLICE2FREEZE) --ice --include-dir Yellow $(ICECPPFLAGS)

include $(top_srcdir)/config/Make.rules

SLICE2CPPFLAGS	:= --ice --include-dir Yellow $(SLICE2CPPFLAGS)
CPPFLAGS	:= -I.. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(VERSIONED_SNAME): $(SERVICE_OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(SERVICE_OBJS)

$(ADMIN): $(AOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(AOBJS) -lYellow $(LIBS) -lreadline -lcurses

../Yellow/StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp: $(SLICE2FREEZE)
	rm -f StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp
	$(SLICE2FREEZECMD) --dict Yellow::StringObjectProxySeqDict,string,Ice::ObjectProxySeq \
	StringObjectProxySeqDict $(slicedir)/Ice/BuiltinSequences.ice

clean::
	rm -f StringObjectProxySeqDict.h StringObjectProxySeqDict.cpp

# Needed for make -jn to work.
../Yellow/Grammar.y: Grammar.h

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

include .depend
