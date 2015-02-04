# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

MAXWARN         = yes

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= Filesystem.o

COBJS		= $(SLICE_OBJS) \
		  Client.o \
		  Grammar.o \
		  Parser.o \
		  Scanner.o

SOBJS		= $(SLICE_OBJS) \
		  FilesystemDB.o \
		  FilesystemI.o \
		  IdentityDirectoryEntryMap.o \
		  IdentityFileEntryMap.o \
		  Server.o

OBJS		= $(COBJS) \
		  $(SOBJS)

all:: IdentityFileEntryMap.cpp IdentityDirectoryEntryMap.cpp

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS) $(READLINE_FLAGS)
SLICE2CPPFLAGS	:= -I. $(SLICE2CPPFLAGS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(LIBS) $(READLINE_LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(DB_RPATH_LINK) -lFreeze $(LIBS)

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
	-rm -f FilesystemDB.h FilesystemDB.cpp
	-rm -rf db/*
