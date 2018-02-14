# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,IcePatch2,$(VERSION))
SONAME     	= $(call mksoname,IcePatch2,$(SOVERSION))
LIBNAME		= $(call mklibname,IcePatch2)

TARGETS   	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

SLICE_OBJS	= FileInfo.o \
		  FileServer.o

OBJS		= ClientUtil.o \
		  Util.o \
		  $(SLICE_OBJS)

HDIR		= $(headerdir)/IcePatch2
SDIR		= $(slicedir)/IcePatch2

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. $(CPPFLAGS) $(BZIP2_FLAGS)

ifeq ($(GCC_COMPILER),yes)
        CPPFLAGS := $(CPPFLAGS) -Wno-deprecated-declarations
endif


SLICE2CPPFLAGS	:= --ice --include-dir IcePatch2 --dll-export ICE_PATCH2_API $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil $(BZIP2_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)$(cpp11libdirsuffix)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(cpp11sonamedir)$(SONAME) $@

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
