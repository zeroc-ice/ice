# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

top_srcdir	= ../..

NAME		= $(top_srcdir)/bin/slice2wsdl

TARGETS		= $(NAME)

OBJS		= Gen.o \
		  Main.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(VOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)  -I$(XERCESC_HOME)/include/xercesc

$(NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) -lSlice $(BASELIBS)

include .depend
