# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceStorm,$(VERSION))
SONAME          = $(call mksoname,IceStorm,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceStorm)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= IceStorm.o \
		  Metrics.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/IceStorm.ice \
		  $(SDIR)/Metrics.ice

HDIR		= $(headerdir)/IceStorm
SDIR		= $(slicedir)/IceStorm

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -I..
ICECPPFLAGS	:= $(ICECPPFLAGS) -I..
SLICE2CPPFLAGS	:= --ice --include-dir IceStorm --checksum --dll-export ICE_STORM_LIB_API $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

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
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
