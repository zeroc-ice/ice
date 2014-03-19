# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceWS,$(VERSION))
SONAME          = $(call mksoname,IceWS,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceWS)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= AcceptorI.o \
                  ConnectorI.o \
		  EndpointInfo.o \
		  ConnectionInfo.o \
                  EndpointI.o \
                  Instance.o \
                  PluginI.o \
                  TransceiverI.o \
                  Util.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/EndpointInfo.ice \
		  $(SDIR)/ConnectionInfo.ice

HDIR		= $(headerdir)/IceWS
SDIR            = $(slicedir)/IceWS

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_WS_API_EXPORTS $(OPENSSL_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IceWS --dll-export ICE_WS_API $(SLICE2CPPFLAGS)

LINKWITH        := $(BZIP2_RPATH_LINK) -lIce -lIceUtil $(OPENSSL_LIBS) $(CXXLIBS)

ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@
endif

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
