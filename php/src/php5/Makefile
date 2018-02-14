# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

include $(top_srcdir)/config/Make.rules.php

CPPFLAGS	:= -I. $(CPPFLAGS) $(ICE_FLAGS) $(shell $(PHP_CONFIG) --includes)

LINKWITH        := $(ICE_LIBS) $(CXXLIBS)

$(libdir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mkplugin,$@,$(SONAME),$(OBJS),$(LINKWITH))

install:: all
	$(call installphplib,$(libdir)/$(LIBNAME),$(DESTDIR)$(install_libdir))
