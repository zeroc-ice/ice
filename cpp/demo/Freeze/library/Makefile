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

OBJS		= Library.o

COBJS		= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  RunParser.o \
		  Client.o \

SOBJS		= LibraryI.o \
		  LibraryTypes.o \
		  BookFactory.o \
		  Server.o

COLOBJS		= Grammar.o \
		  Scanner.o \
		  Parser.o \
		  RunParser.o \
		  LibraryI.o \
		  LibraryTypes.o \
		  BookFactory.o \
		  Collocated.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

SLICE_SRCS	= Library.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(DB_FLAGS) $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice $(SLICE2CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) -lreadline -lcurses

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(COLLOCATED): $(OBJS) $(COLOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COLOBJS) -lFreeze -lIceXML $(LIBS) $(DB_LIBS) -lreadline -lcurses

LibraryTypes.h LibraryTypes.cpp: Library.ice $(SLICE2FREEZE)
	rm -f LibraryTypes.h LibraryTypes.cpp
	$(SLICE2FREEZE) --ice -I$(slicedir) --dict StringIsbnSeqDict,string,Ice::StringSeq LibraryTypes $(slicedir)/Ice/BuiltinSequences.ice Library.ice

clean::
	rm -f LibraryTypes.h LibraryTypes.cpp

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

include .depend
