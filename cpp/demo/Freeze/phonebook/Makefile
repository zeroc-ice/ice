# **********************************************************************
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

CPPFLAGS	:= -I. -I$(DB_HOME)/include $(CPPFLAGS)
LDFLAGS		:= $(LDFLAGS) -L$(DB_HOME)/lib

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lFreeze -lIceXML $(LIBS) -ldb -lreadline -lcurses

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) -lFreeze -lIceXML $(LIBS) -ldb

$(COLLOCATED): $(OBJS) $(COLOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COLOBJS) -lFreeze -lIceXML $(LIBS) -ldb -lreadline -lcurses

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

PhoneBook.h PhoneBook.cpp: PhoneBook.ice $(SLICE2CPP)
	rm -f PhoneBook.h PhoneBook.cpp
	$(SLICE2CPP) $(ICECPPFLAGS) PhoneBook.ice

clean::
	rm -f PhoneBook.h PhoneBook.cpp

NameIdentitiesDict.h NameIdentitiesDict.cpp: PhoneBook.ice $(SLICE2FREEZE)
	rm -f NameIdentitiesDict.h NameIdentitiesDict.cpp
	$(SLICE2FREEZE) $(ICECPPFLAGS) --dict NameIdentitiesDict,string,Identities NameIdentitiesDict PhoneBook.ice

clean::
	rm -f NameIdentitiesDict.h NameIdentitiesDict.cpp

include .depend
