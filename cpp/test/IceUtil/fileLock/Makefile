# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)
CLIENTF		= $(call mktestname,clientFail)

TARGETS		= $(CLIENT) $(CLIENTF)

OBJS		= Client.o
OBJFS		= ClientFail.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(CLIENT): $(OBJS)
	rm -f $@
	$(call mktest,$@,$(OBJS),$(BASELIBS))


$(CLIENTF): $(OBJFS)
	rm -f $@
	$(call mktest,$@,$(OBJFS),$(BASELIBS))

clean::
	-rm -f run.pid

include .depend
