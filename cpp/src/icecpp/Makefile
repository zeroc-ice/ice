# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

NAME		= $(top_srcdir)/bin/icecpp

TARGETS		= $(NAME)

OBJS		= cccp.o \
		  cexp.o \
		  prefix.o

include $(top_srcdir)/config/Make.rules

CFLAGS += -I. -DPREFIX=\"\"

cexp.c: cexp.y
	bison -o cexp.c cexp.y

$(NAME): $(OBJS)
	rm -f $@
	$(CC) $(CFLAGS) -o $@ $(OBJS)

install:: all
	$(call installprogram,$(NAME),$(install_bindir))

clean::
	rm -f cexp.c
