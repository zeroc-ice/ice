# **********************************************************************
#
# Copyright (c) 2003
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

top_srcdir	= ../../..

CLIENT		= client

NAME		= libHelloService.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

TARGETS		= $(CLIENT) $(NAME) $(VERSIONED_NAME)

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

$(VERSIONED_NAME): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS)  -o $@ $(OBJS) $(SOBJS)

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

include .depend
