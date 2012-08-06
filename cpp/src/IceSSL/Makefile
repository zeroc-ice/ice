
# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules.common

ifeq ($(MinGW), yes)
	DLLNAME         = $(top_srcdir)/bin/icessl$(COMPSUFFIX)$(SOVERSION)$(LIBSUFFIX).dll
	TARGETS		= $(DLLNAME) $(LIBNAME)
else
	LIBFILENAME     = $(call mklibfilename,IceSSL,$(VERSION))
	SONAME          = $(call mksoname,IceSSL,$(SOVERSION))  
	LIBNAME		= $(call mklibname,IceSSL)
	TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
endif

OBJS		= AcceptorI.o \
		  Certificate.o \
                  ConnectorI.o \
		  EndpointInfo.o \
		  ConnectionInfo.o \
                  EndpointI.o \
                  Instance.o \
                  PluginI.o \
                  TransceiverI.o \
                  Util.o \
		  RFC2253.o \
		  TrustManager.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/EndpointInfo.ice \
		  $(SDIR)/ConnectionInfo.ice

HDIR		= $(headerdir)/IceSSL
SDIR            = $(slicedir)/IceSSL

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_SSL_API_EXPORTS $(OPENSSL_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IceSSL --dll-export ICE_SSL_API $(SLICE2CPPFLAGS)

ifeq ($(MinGW), yes)

LINKWITH        := -lice$(COMPSUFFIX)$(SOVERSION)$(LIBSUFFIX) -liceutil$(COMPSUFFIX)$(SOVERSION)$(LIBSUFFIX) $(ICE_OS_LIBS) $(OPENSSL_LIBS)
	
$(DLLNAME): $(OBJS)
	$(CXX) -shared $(LDFLAGS) -o $(DLLNAME) $(OBJS) $(LINKWITH)

install:: all
	$(call installprogram,$(DLLNAME),$(install_bindir))

else
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
endif

include .depend
