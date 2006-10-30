# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME		= $(call mkrubylibname,IceRuby)
SONAME          = $(LIBNAME)

TARGETS		= $(rubydir)/$(LIBNAME)

OBJS		= Communicator.o \
		  Init.o \
		  ImplicitContext.o \
		  Logger.o \
		  ObjectFactory.o \
		  Operation.o \
		  Properties.o \
		  Proxy.o \
		  Slice.o \
		  Types.o \
		  Util.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. $(CPPFLAGS) $(ICE_FLAGS) $(RUBY_FLAGS)

LINKWITH        := -L$(libdir) $(ICE_LIBS) $(RUBY_LIBS) $(CXXLIBS)

$(rubydir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

install:: all
	$(call installrubylib,$(rubydir)/$(LIBNAME),$(install_rubydir))

include .depend
