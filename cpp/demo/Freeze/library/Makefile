# **********************************************************************
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

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) -lreadline -lcurses

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) -lFreeze -lIceXML $(LIBS) $(DB_LIBS)

$(COLLOCATED): $(OBJS) $(COLOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COLOBJS) -lFreeze -lIceXML $(LIBS) $(DB_LIBS) -lreadline -lcurses

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

Library.h Library.cpp: Library.ice $(SLICE2CPP)
	rm -f Library.h Library.cpp
	$(SLICE2CPP) --ice -I$(slicedir) Library.ice

clean::
	rm -f Library.h Library.cpp

LibraryTypes.h LibraryTypes.cpp: Library.ice $(SLICE2FREEZE)
	rm -f LibraryTypes.h LibraryTypes.cpp
	$(SLICE2FREEZE) --ice -I$(slicedir) --dict StringIsbnSeqDict,string,Ice::StringSeq LibraryTypes $(slicedir)/Ice/BuiltinSequences.ice Library.ice

clean::
	rm -f LibraryTypes.h LibraryTypes.cpp

include .depend
