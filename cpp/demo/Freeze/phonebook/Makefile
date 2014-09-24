
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server
COLLOCATED	= collocated

TARGETS		= $(CLIENT) $(SERVER) $(COLLOCATED)

SLICE_OBJS	= PhoneBook.o

COBJS		= $(SLICE_OBJS) \
		  Client.o \
		  Grammar.o \
		  Parser.o \
		  RunParser.o \
		  Scanner.o

SOBJS		= $(SLICE_OBJS) \
		  ContactFactory.o \
		  NameIndex.o \
		  PhoneBookI.o \
		  Server.o

COLOBJS		= $(SLICE_OBJS) \
		  Collocated.o \
		  ContactFactory.o \
		  Grammar.o \
		  NameIndex.o \
		  Parser.o \
		  PhoneBookI.o \
		  RunParser.o \
		  Scanner.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(COLOBJS)

all:: NameIndex.cpp NameIndex.h

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS) $(READLINE_FLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(LIBS) $(READLINE_LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

$(COLLOCATED): $(COLOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COLOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS) $(READLINE_LIBS)

# The slice2freeze rules are structured like this to avoid issues with
# parallel make.
NameIndex.h: NameIndex.cpp
NameIndex.cpp: PhoneBook.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f NameIndex.h NameIndex.cpp
	$(SLICE2FREEZE) -I. $(ICECPPFLAGS) --index NameIndex,Demo::Contact,name,case-insensitive NameIndex PhoneBook.ice

clean::
	-rm -f NameIndex.h NameIndex.cpp
	-rm -rf db/*
