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

LIBFILENAME     = $(call mklibfilename,HelloService)
SONAME          = $(call mksoname,HelloService)  

TARGETS		= $(CLIENT) $(LIBFILENAME)

SLICE_OBJS	= Hello.o

COBJS		= $(SLICE_OBJS) \
		  Client.o

SOBJS		= $(SLICE_OBJS) \
		  HelloI.o \
		  HelloServiceI.o

OBJS		= $(COBJS) \
		  $(SOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)
LINKWITH	:= -lIceBox $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(LIBFILENAME): $(SOBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(SOBJS),$(LINKWITH))

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(LIBS)

clean::
	-rm -rf db/node/* db/registry/*
