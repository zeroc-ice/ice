# **********************************************************************
#
# Copyright (c) 2002
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

NAME		= libHelloService.so
VERSIONED_NAME	= $(NAME).$(VERSION)

TARGETS		= $(CLIENT) $(NAME) $(VERSIONED_NAME)

OBJS		= Hello.o

COBJS		= Client.o

SOBJS		= HelloI.o \
		  HelloServiceI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

$(VERSIONED_NAME): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $(OBJS) $(SOBJS)

$(NAME): $(VERSIONED_NAME)
	rm -f $@
	ln -s $(VERSIONED_NAME) $@

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

Hello.h Hello.cpp: Hello.ice $(SLICE2CPP)
	rm -f Hello.h Hello.cpp
	$(SLICE2CPP) Hello.ice

clean::
	rm -f Hello.h Hello.cpp

include .depend
