# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceSSL,$(VERSION))
SONAME          = $(call mksoname,IceSSL,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceSSL)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= AcceptorI.o \
		  Certificate.o \
                  ConnectorI.o \
                  EndpointI.o \
                  Instance.o \
                  PluginI.o \
                  TransceiverI.o \
                  Util.o \
		  RFC2253.o \
		  TrustManager.o

SRCS		= $(OBJS:.o=.cpp)

HDIR		= $(includedir)/IceSSL

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_SSL_API_EXPORTS $(OPENSSL_FLAGS)

LINKWITH        := $(BZIP2_RPATH_LINK) -lIce -lIceUtil $(OPENSSL_LIBS) $(CXXLIBS)

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
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
