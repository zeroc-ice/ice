
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

OBJS		= PhoneBook.o

COBJS		= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  RunParser.o \
		  Client.o \

SOBJS		= PhoneBookI.o \
		  ContactFactory.o \
                  NameIndex.o \
		  Server.o

COLOBJS		= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  RunParser.o \
		  PhoneBookI.o \
		  ContactFactory.o \
                  NameIndex.o \
		  Collocated.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

SLICE_SRCS	= PhoneBook.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(READLINE_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) $(READLINE_LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

$(COLLOCATED): $(OBJS) $(COLOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COLOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS) $(READLINE_LIBS)

NameIndex.h NameIndex.cpp: PhoneBook.ice $(SLICE2FREEZE)
	rm -f NameIndex.h NameIndex.cpp
	$(SLICE2FREEZE) $(ICECPPFLAGS) --index NameIndex,Demo::Contact,name,case-insensitive NameIndex PhoneBook.ice

clean::
	rm -f NameIndex.h NameIndex.cpp

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp
	rm -f db/contacts db/log*

include .depend
