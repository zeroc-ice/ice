# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME		= $(call mkphplibname,IcePHP)
SONAME          = $(LIBNAME)

TARGETS		= $(libdir)/$(LIBNAME)

OBJS		= Communicator.o \
		  Init.o \
		  Marshal.o \
		  Profile.o \
		  Proxy.o \
		  Util.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS) $(ICE_FLAGS) $(PHP_FLAGS)

LINKWITH        := $(ICE_LIBS) $(PHP_LIBS) $(CXXLIBS)

$(libdir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

install:: all
	$(call installphplib,$(libdir)/$(LIBNAME),$(install_libdir))

include .depend
