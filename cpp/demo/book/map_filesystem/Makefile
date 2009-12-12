# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

COBJS		= Client.o \
		  Parser.o \
		  Grammar.o \
		  Scanner.o

SOBJS		= IdentityNodeMap.o \
		  PersistentFilesystem.o \
		  PersistentFilesystemI.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(COLOBJS:.o=.cpp)

SLICE_SRCS	= Filesystem.ice PersistentFilesystem.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(READLINE_FLAGS) $(CPPFLAGS)
SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) $(READLINE_LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

IdentityNodeMap.h: IdentityNodeMap.cpp
IdentityNodeMap.cpp: PersistentFilesystem.ice Filesystem.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IdentityNodeMap.h IdentityNodeMap.cpp
	$(SLICE2FREEZE) -I$(slicedir) -I. --ice --dict IdentityNodeMap,Ice::Identity,Filesystem::PersistentNode \
		IdentityNodeMap Filesystem.ice PersistentFilesystem.ice  $(slicedir)/Ice/Identity.ice
clean::
	-rm -f db/*
	-rm -f IdentityNodeMap.h IdentityNodeMap.cpp

include .depend
