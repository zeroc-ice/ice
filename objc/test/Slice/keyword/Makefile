# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Touch is licensed to you under the terms described in the
# ICE_TOUCH_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

TARGETS		= $(CLIENT)

SLICE_OBJS	= Key.o Inherit.o

OBJS		= Key.o \
			  Inherit.o \
			  Client.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(CC) $(LDFLAGS) $(LDEXEFLAGS) $(STATICLIBSTDFLAG) -o $@ $(OBJS) $(TEST_LIBS)

