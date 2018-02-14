# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mkpylibfilename,IcePy,$(VERSION))
SONAME          = $(call mkpysoname,IcePy,$(SOVERSION))
LIBNAME		= $(call mkpylibname,IcePy)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= BatchRequestInterceptor.o \
	      	  Communicator.o \
		  Connection.o \
		  ConnectionInfo.o \
		  Current.o \
		  Endpoint.o \
		  EndpointInfo.o \
		  ImplicitContext.o \
		  Init.o \
		  Logger.o \
		  ObjectAdapter.o \
		  ObjectFactory.o \
		  Operation.o \
		  Properties.o \
		  PropertiesAdmin.o \
		  Proxy.o \
		  Slice.o \
		  Thread.o \
		  Types.o \
		  Util.o

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS) $(ICE_FLAGS) $(PYTHON_FLAGS)

LINKWITH        := $(ICE_LIBS) $(PYTHON_LIBS) $(CXXLIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
