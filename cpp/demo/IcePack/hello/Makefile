# **********************************************************************
#
# Copyright (c) 2002
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
SERVER		= server

NAME		= libHelloService.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

TARGETS		= $(CLIENT) $(NAME) $(VERSIONED_NAME) $(SERVER)

OBJS		= Hello.o

COBJS		= Client.o

SVC_OBJS	= HelloI.o \
		  HelloServiceI.o

SVR_OBJS	= HelloI.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SVC_OBJS:.o=.cpp) \
		  $(SVR_OBJS:.o=.cpp)

SLICE_SRCS	= Hello.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS) $(SVC_OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS)  -o $@ $(OBJS) $(SVC_OBJS) -lIcePack

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) -lIcePack

$(SERVER): $(OBJS) $(SVR_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SVR_OBJS) $(LIBS) -lIcePack

include .depend
