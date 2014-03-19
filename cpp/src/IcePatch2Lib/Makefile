# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IcePatch2,$(VERSION))
SONAME          = $(call mksoname,IcePatch2,$(SOVERSION))  
LIBNAME		= $(call mklibname,IcePatch2)

TARGETS         = $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= Util.o \
		  ClientUtil.o \
		  FileInfo.o \
		  FileServer.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/FileInfo.ice \
		  $(SDIR)/FileServer.ice

HDIR		= $(headerdir)/IcePatch2
SDIR		= $(slicedir)/IcePatch2

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I.. $(CPPFLAGS) $(OPENSSL_FLAGS) $(BZIP2_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir IcePatch2 --dll-export ICE_PATCH2_API $(SLICE2CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil $(OPENSSL_LIBS) $(BZIP2_LIBS)

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
