# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mkrubylibfilename,IceRuby,$(VERSION))
SONAME          = $(call mkrubysoname,IceRuby,$(SOVERSION))  
LIBNAME		= $(call mkrubylibname,IceRuby)

TARGETS		= $(call mklibtargets,$(rubydir)/$(LIBFILENAME),$(rubydir)/$(SONAME),$(rubydir)/$(LIBNAME))

OBJS		= Communicator.o \
		  Init.o \
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

$(rubydir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(rubydir)/$(SONAME): $(rubydir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(rubydir)/$(LIBNAME): $(rubydir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(call installlib,$(install_rubydir),$(rubydir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
