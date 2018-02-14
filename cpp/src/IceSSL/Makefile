
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,IceSSL,$(VERSION))
SONAME		= $(call mksoname,IceSSL,$(SOVERSION))
LIBNAME		= $(call mklibname,IceSSL)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

SLICE_OBJS	= ConnectionInfo.o \
		  EndpointInfo.o

OBJS		= AcceptorI.o \
		  Certificate.o \
		  ConnectorI.o \
		  EndpointI.o \
		  Instance.o \
		  PluginI.o \
		  RFC2253.o \
		  SSLEngine.o \
		  TrustManager.o \
		  Util.o \
		  $(SLICE_OBJS)

ifneq ($(findstring MINGW,$(shell uname)),)
OBJS		:= $(OBJS) \
		   SChannelEngine.o \
		   SChannelTransceiverI.o
else
ifeq ($(shell uname),Darwin)
OBJS		:= $(OBJS) \
		   SecureTransportEngine.o \
		   SecureTransportTransceiverI.o
else
OBJS		:= $(OBJS) \
		   OpenSSLEngine.o \
		   OpenSSLTransceiverI.o
endif
endif

HDIR		= $(headerdir)/IceSSL
SDIR		= $(slicedir)/IceSSL

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_SSL_API_EXPORTS $(OPENSSL_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IceSSL --dll-export ICE_SSL_API $(SLICE2CPPFLAGS)

LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil $(SSL_OS_LIBS) $(CXXLIBS)

ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)$(cpp11libdirsuffix)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)$(cpp11libdirsuffix)
	rm -f $@
	ln -s $(cpp11sonamedir)$(SONAME) $@
endif

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
