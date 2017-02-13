# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

SLICE_OBJS	= ServicesTest.o

COBJS		= Client.o \
		  AllTests.o

OBJS		= $(COBJS) $(SLICE_OBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LIBS		:= -lGlacier2ObjC$(libsuffix) -lIceStormObjC$(libsuffix) -lIceGridObjC$(libsuffix) $(TEST_LIBS)

$(CLIENT): $(COBJS) $(SLICE_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(SLICE_OBJS) $(TEST_LIBS)
