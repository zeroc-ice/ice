# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

LIBFILENAME     = $(call mklibfilename,HelloService)
SONAME          = $(call mksoname,HelloService)  

TARGETS		= $(CLIENT) $(LIBFILENAME)

OBJS		= Hello.o

COBJS		= Client.o

SOBJS		= HelloI.o \
		  HelloServiceI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Hello.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)
LINKWITH	:= -lIceBox $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(LIBFILENAME): $(OBJS) $(SOBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS) $(SOBJS),$(LINKWITH))

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

include .depend
