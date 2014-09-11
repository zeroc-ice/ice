# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
		  Connection.o \
		  Endpoint.o \
		  Init.o \
		  Logger.o \
		  Operation.o \
		  Properties.o \
		  Proxy.o \
		  Types.o \
		  Util.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules.php

CPPFLAGS	:= -I. $(CPPFLAGS) $(ICE_FLAGS) $(PHP_FLAGS) -DPHP_DEBUG

LINKWITH        := $(ICE_LIBS) $(PHP_LIBS) $(CXXLIBS)

$(libdir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

install:: all
	$(call installphplib,$(libdir)/$(LIBNAME),$(DESTDIR)$(install_libdir))

depend:: $(SRCS)
	-rm -f .depend .depend.mak
	if test -n "$(SRCS)" ; then \
	    $(CXX) -DMAKEDEPEND -M $(CXXFLAGS) $(CPPFLAGS) $(SRCS) | $(ice_dir)/config/makedepend.py; \
	fi

include .depend
