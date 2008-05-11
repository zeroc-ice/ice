# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Filesystem.o

COBJS		= Scanner.o \
		  Grammar.o \
		  Parser.o \
		  Client.o

SOBJS		= FilesystemI.o \
		  Server.o \
		  IdentityNodeMap.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(READLINE_FLAGS) $(CPPFLAGS)
SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) $(READLINE_LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

Scanner.cpp: Scanner.l
	flex Scanner.l
	rm -f $@
	echo #include "IceUtil/Config.h" > Scanner.cpp
	cat lex.yy.c >> Scanner.cpp
	rm -f lex.yy.c

Grammar.cpp Grammar.h: Grammar.y
	rm -f Grammar.h Grammar.cpp
	bison -dvt Grammar.y
	mv Grammar.tab.c Grammar.cpp
	mv Grammar.tab.h Grammar.h
	rm -f Grammar.output

IdentityNodeMap.h IdentityNodeMap.cpp: $(SLICE2FREEZE) $(SLICE_GEN)
	rm -f IdentityNodeMap.h IdentityNodeMap.cpp
	$(SLICE2FREEZE) --ice -I$(top_srcdir)/../slice \
	    --dict IdentityNodeMap,Ice::Identity,Filesystem::PersistentNode \
	    IdentityNodeMap Filesystem.ice $(top_srcdir)/../slice/Ice/Identity.ice

clean::
	-rm -f db/*
	-rm -f IdentityNodeMap.cpp IdentityNodeMap.h
	-rm -f Grammar.cpp Grammar.h
	-rm -f Scanner.cpp

include .depend
