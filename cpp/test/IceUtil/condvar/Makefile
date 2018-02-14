# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

WQ		= $(call mktestname,workqueue)
MATCH		= $(call mktestname,match)

TARGETS		= $(WQ) $(MATCH)

OBJS		= WorkQueue.o \
		  Match.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)

$(WQ): WorkQueue.o
	rm -f $@
	$(call mktest,$@,WorkQueue.o,$(BASELIBS))

$(MATCH): Match.o
	rm -f $@
	$(call mktest,$@,Match.o,$(BASELIBS))
