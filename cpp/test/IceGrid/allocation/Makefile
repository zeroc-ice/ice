# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server
VERIFIER	= verifier

TARGETS		= $(CLIENT) $(SERVER) $(VERIFIER)

SLICE_OBJS	= Test.o

COBJS		= $(SLICE_OBJS) \
		  Client.o \
		  AllTests.o

SOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Server.o

VOBJS		= PermissionsVerifier.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(VOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) -lIceGrid -lGlacier2 $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(LIBS)

$(VERIFIER): $(VOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(VOBJS) -lGlacier2 $(LIBS)

clean::
	-rm -rf db/node db/registry db/node-1 db/node-2 db/replica-*
