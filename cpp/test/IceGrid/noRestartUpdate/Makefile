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

SVCFILENAME     = $(call mklibfilename,TestService)
SVCSONAME	= $(call mksoname,TestService) 

TARGETS		= $(CLIENT) $(SERVER) $(SVCFILENAME) 

SLICE_OBJS	= Test.o

COBJS		= $(SLICE_OBJS) \
		  Client.o \
		  AllTests.o

SOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Server.o

SERVICE_OBJS	= $(SLICE_OBJS) \
		  TestI.o \
		  Service.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SERVICE_OBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LINKWITH	:= -lIceBox $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) -lIceGrid -lGlacier2 -lIceBox $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(LIBS)

$(SVCFILENAME): $(SERVICE_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SVCSONAME),$(SERVICE_OBJS), $(LINKWITH))

clean::
	-rm -rf db/node db/registry db/node-1 db/node-2 db/replica-*
