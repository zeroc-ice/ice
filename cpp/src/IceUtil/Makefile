# ********************************************************************** #
# Copyright (c) 2001
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../..

BASE		= libIceUtil.so
VERSIONED_BASE	= $(BASE).$(VERSION)

NAME		= $(top_srcdir)/lib/$(BASE)
VERSIONED_NAME	= $(top_srcdir)/lib/$(VERSIONED_BASE)

TARGETS		= $(NAME) $(VERSIONED_NAME)

OBJS		= Exception.o \
		  Unicode.o \
		  UUID.o \
		  RecMutex.o \
		  RWRecMutex.o \
		  Cond.o \
		  Thread.o \
		  ThreadException.o \
		  Time.o \
		  InputUtil.o \
		  OutputUtil.o \
                  Base64.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) -Xlinker -soname -Xlinker $(VERSIONED_BASE) $(LDFLAGS) -shared -o $@ $(OBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_BASE) $@

include .depend
