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

CPPFLAGS	:= -I. $(CPPFLAGS) $(XERCESC_FLAGS)

$(NAME): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lSlice $(BASELIBS)

$(VALIDATE): $(VOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(VOBJS) $(BASELIBS) $(XERCESC_LIBS)

install:: all
	$(INSTALL_PROGRAM) $(NAME) $(install_bindir)
	$(INSTALL_PROGRAM) $(VALIDATE) $(install_bindir)

include .depend
