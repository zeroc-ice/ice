# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= PhoneBook.o

COBJS		= Grammer.o \
		  Scanner.o \
		  Parser.o \
		  Client.o \

SOBJS		= PhoneBookI.o \
		  ServantFactory.o \
		  Evictor.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS) -I$(DB)/include
LDFLAGS		:= $(LDFLAGS) -L$(DB)/lib

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) -lreadline -lcurses

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) -lFreeze -ldb $(LIBS)

Grammer.cpp Grammer.h: Grammer.y
	bison -dvt Grammer.y
	rm -f Grammer.cpp ; mv Grammer.tab.c Grammer.cpp
	rm -f Grammer.h ; mv Grammer.tab.h Grammer.h

Scanner.cpp: Scanner.l Grammer.h
	flex Scanner.l
	rm -f Scanner.cpp ; mv lex.yy.c Scanner.cpp

clean::
	-rm -f Grammer.cpp Grammer.h Grammer.output
	-rm -f Scanner.cpp lex.yy.c

PhoneBook.h PhoneBook.cpp: PhoneBook.ice $(SLICE)
	rm -f PhoneBook.h PhoneBook.cpp
	$(SLICE) PhoneBook.ice

clean::
	rm -f PhoneBook.h PhoneBook.cpp

include .depend
