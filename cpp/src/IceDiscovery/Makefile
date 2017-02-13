# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceDiscovery,$(VERSION))
SONAME          = $(call mksoname,IceDiscovery,$(SOVERSION))
LIBNAME		= $(call mklibname,IceDiscovery)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

SLICE_OBJS      = IceDiscovery.o

OBJS		= LocatorI.o \
		  LookupI.o \
		  PluginI.o \
		  $(SLICE_OBJS)

HDIR		= $(headerdir)/IceDiscovery
SDIR		= $(slicedir)/IceDiscovery

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_DISCOVERY_API_EXPORTS
SLICE2CPPFLAGS	:= --ice --include-dir IceDiscovery $(SLICE2CPPFLAGS)

LINKWITH        := -lIce -lIceUtil $(CXXLIBS)

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

../IceDiscovery/IceDiscovery.h IceDiscovery.cpp: \
	$(slicedir)/IceDiscovery/IceDiscovery.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	rm -f IceDiscovery.h IceDiscovery.cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(slicedir)/IceDiscovery/IceDiscovery.ice
	@touch IceDiscovery.cpp
	@mkdir -p .depend
	@$(SLICE2CPP) $(SLICE2CPPFLAGS) --depend $(slicedir)/IceDiscovery/IceDiscovery.ice > .depend/IceDiscovery.ice.d

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
