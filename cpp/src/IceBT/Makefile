
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,IceBT,$(VERSION))
SONAME		= $(call mksoname,IceBT,$(SOVERSION))
LIBNAME		= $(call mklibname,IceBT)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

SLICE_OBJS	= ConnectionInfo.o \
		  EndpointInfo.o \
		  Types.o

OBJS		= AcceptorI.o \
		  ConnectorI.o \
		  DBus.o \
		  EndpointI.o \
		  Engine.o \
		  Instance.o \
		  PluginI.o \
		  StreamSocket.o \
		  TransceiverI.o \
		  Util.o \
		  $(SLICE_OBJS)

HDIR		= $(headerdir)/IceBT
SDIR  		= $(slicedir)/IceBT

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_BT_API_EXPORTS `pkg-config --cflags dbus-1`
SLICE2CPPFLAGS	:= --ice --include-dir IceBT --dll-export ICE_BT_API $(SLICE2CPPFLAGS)

LINKWITH        := $(BZIP2_RPATH_LINK) -lIce -lIceXML -lIceUtil `pkg-config --libs dbus-1` $(CXXLIBS)

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
