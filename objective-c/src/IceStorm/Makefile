# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,IceStormObjC,$(VERSION))
SONAME		= $(call mksoname,IceStormObjC,$(SOVERSION))
LIBNAME		= $(call mklibname,IceStormObjC)

TARGETS 	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

SLICE_OBJS  	= Metrics.o \
		  IceStorm.o

OBJS		= $(SLICE_OBJS)

HDIR		= $(headerdir)/objc/IceStorm
SDIR		= $(slicedir)/IceStorm

include $(top_srcdir)/config/Make.rules

SLICE2OBJCFLAGS	:= --ice --include-dir objc/IceStorm --dll-export ICE_STORM_API $(SLICE2OBJCFLAGS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LIBS))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
