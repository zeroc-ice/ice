# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceLocatorDiscovery,$(VERSION))
SONAME          = $(call mksoname,IceLocatorDiscovery,$(SOVERSION))
LIBNAME		= $(call mklibname,IceLocatorDiscovery)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

SLICE_OBJS      = IceLocatorDiscovery.o

OBJS		= PluginI.o \
		  $(SLICE_OBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_LOCATOR_DISCOVERY_API_EXPORTS
SLICE2CPPFLAGS	:= --ice --include-dir IceLocatorDiscovery $(SLICE2CPPFLAGS)

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

../IceLocatorDiscovery/%.h %.cpp: $(slicedir)/IceLocatorDiscovery/%.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	rm -f $(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(slicedir)/IceLocatorDiscovery/$(*F).ice
	@touch $(*F).cpp
	@mkdir -p .depend
	@$(SLICE2CPP) $(SLICE2CPPFLAGS) --depend $(slicedir)/IceLocatorDiscovery/$(*F).ice > .depend/$(*F).ice.d

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
