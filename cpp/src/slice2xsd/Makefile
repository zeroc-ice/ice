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

NAME		= $(top_srcdir)/bin/slice2xsd
VALIDATE	= $(top_srcdir)/bin/xsdvalidate

TARGETS		= $(NAME) $(VALIDATE)

OBJS		= Gen.o \
		  Main.o

VOBJS		= Validate.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(VOBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)  -I$(XERCESC_HOME)/include/xercesc

$(NAME): $(OBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(OBJS) -lSlice $(BASELIBS)

$(VALIDATE): $(VOBJS)
	rm -f $@
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(VOBJS) $(BASELIBS) -lxerces-c1_5_2

include .depend
