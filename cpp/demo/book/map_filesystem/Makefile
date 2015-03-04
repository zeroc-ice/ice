# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

SOBJS		= FilesystemDB.o \
		  IdentityFileEntryMap.o \
		  IdentityDirectoryEntryMap.o \
		  FilesystemI.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Filesystem.ice FilesystemDB.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS) $(READLINE_FLAGS)
SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) $(READLINE_LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

IdentityFileEntryMap.h: IdentityFileEntryMap.cpp
IdentityFileEntryMap.cpp: FilesystemDB.ice Filesystem.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IdentityFileEntryMap.h IdentityFileEntryMap.cpp
	$(SLICE2FREEZE) -I$(slicedir) -I. --ice \
	    --dict FilesystemDB::IdentityFileEntryMap,Ice::Identity,FilesystemDB::FileEntry \
	    IdentityFileEntryMap FilesystemDB.ice  $(slicedir)/Ice/Identity.ice
clean::
	-rm -f IdentityFileEntryMap.h IdentityFileEntryMap.cpp

IdentityDirectoryEntryMap.h: IdentityDirectoryEntryMap.cpp
IdentityDirectoryEntryMap.cpp: FilesystemDB.ice Filesystem.ice $(SLICE2FREEZE) $(SLICEPARSERLIB)
	rm -f IdentityDirectoryEntryMap.h IdentityDirectoryEntryMap.cpp
	$(SLICE2FREEZE) -I$(slicedir) -I. --ice \
	    --dict FilesystemDB::IdentityDirectoryEntryMap,Ice::Identity,FilesystemDB::DirectoryEntry \
	    IdentityDirectoryEntryMap FilesystemDB.ice  $(slicedir)/Ice/Identity.ice
clean::
	-rm -f IdentityDirectoryEntryMap.h IdentityDirectoryEntryMap.cpp

clean::
	-rm -rf db/*

include .depend
