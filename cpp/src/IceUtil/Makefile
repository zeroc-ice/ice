# ********************************************************************** #
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
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
		  Time.o \
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
