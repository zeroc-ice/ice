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

LIBFILENAME     = $(call mklibfilename,HelloService,$(VERSION))
SONAME          = $(call mksoname,HelloService,$(SOVERSION))  
LIBNAME         = $(call mklibname,HelloService)  

TARGETS		= $(CLIENT) $(LIBFILENAME) $(SONAME) $(LIBNAME)


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

$(LIBFILENAME): $(OBJS) $(SOBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS) $(SOBJS),$(LINKWITH))

$(SONAME): $(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(LIBNAME): $(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

include .depend
