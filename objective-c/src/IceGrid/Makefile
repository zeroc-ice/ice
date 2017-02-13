# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,IceGridObjC,$(VERSION))
SONAME		= $(call mksoname,IceGridObjC,$(SOVERSION))
LIBNAME		= $(call mklibname,IceGridObjC)

TARGETS 	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

SLICE_OBJS	= Admin.o \
		  Descriptor.o \
		  Exception.o \
		  FileParser.o \
		  Locator.o \
		  Observer.o \
		  PluginFacade.o \
		  Query.o \
		  Registry.o \
		  Session.o \
		  UserAccountMapper.o

OBJS		= $(SLICE_OBJS)

HDIR		= $(headerdir)/objc/IceGrid
SDIR		= $(slicedir)/IceGrid

include $(top_srcdir)/config/Make.rules

SLICE2OBJCFLAGS	:= --ice --include-dir objc/IceGrid --dll-export ICE_GRID_API $(SLICE2OBJCFLAGS)
LINKWITH        := -lGlacier2ObjC $(LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
