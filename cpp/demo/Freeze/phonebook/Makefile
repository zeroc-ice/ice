
# **********************************************************************
#
# Copyright (c) 2003
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
		  NameIdentitiesDict.o \
		  ContactFactory.o \
		  Server.o

COLOBJS		= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  RunParser.o \
		  PhoneBookI.o \
		  NameIdentitiesDict.o \
		  ContactFactory.o \
		  Collocated.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

SLICE_SRCS	= PhoneBook.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(DB_FLAGS) $(READLINE_FLAGS) $(CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) $(READLINE_LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(COLLOCATED): $(OBJS) $(COLOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COLOBJS) -lFreeze -lIceXML $(LIBS) $(DB_LIBS) $(READLINE_LIBS)

NameIdentitiesDict.h NameIdentitiesDict.cpp: PhoneBook.ice $(SLICE2FREEZE)
	rm -f NameIdentitiesDict.h NameIdentitiesDict.cpp
	$(SLICE2FREEZE) $(ICECPPFLAGS) --dict NameIdentitiesDict,string,Identities NameIdentitiesDict PhoneBook.ice

clean::
	rm -f NameIdentitiesDict.h NameIdentitiesDict.cpp

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

include .depend
