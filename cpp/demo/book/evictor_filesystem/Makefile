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

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= Filesystem.o PersistentFilesystem.o

COBJS		=  Filesystem.o \
		  Client.o \
		  Grammar.o \
		  Parser.o \
		  Scanner.o

SOBJS		= $(SLICE_OBJS) \
		  PersistentFilesystemI.o \
		  Server.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(COLOBJS)


include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS) $(READLINE_FLAGS)
SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) $(LIBS) $(READLINE_LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

clean::
	-rm -rf db/*
