# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

NAME		= $(top_srcdir)/bin/slice2docbook

TARGETS		= $(NAME)

OBJS		= Gen.o \
		  Main.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../parser $(CPPFLAGS)

$(NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) -lSlice $(BASELIBS)

include .depend
