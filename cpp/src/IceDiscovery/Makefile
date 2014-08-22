# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceDiscovery,$(VERSION))
SONAME          = $(call mksoname,IceDiscovery,$(SOVERSION))
LIBNAME		= $(call mklibname,IceDiscovery)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		=  IceDiscovery.o \
		  LocatorI.o \
		  LookupI.o \
		  PluginI.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(slicedir)/IceDiscovery/IceDiscovery.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_DISCOVERY_API_EXPORTS
SLICE2CPPFLAGS	:= --ice --include-dir IceDiscovery --dll-export ICE_DISCOVERY_API $(SLICE2CPPFLAGS)

LINKWITH        := $(BZIP2_RPATH_LINK) -lIce -lIceUtil $(CXXLIBS)

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

../IceDiscovery/IceDiscovery.h IceDiscovery.cpp: $(slicedir)/IceDiscovery/IceDiscovery.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	rm -f IceDiscovery.h IceDiscovery.cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(slicedir)/IceDiscovery/IceDiscovery.ice

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
