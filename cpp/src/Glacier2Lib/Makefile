# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,Glacier2,$(VERSION))
SONAME          = $(call mksoname,Glacier2,$(SOVERSION))  
LIBNAME		= $(call mklibname,Glacier2)

TARGETS         = $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= PermissionsVerifier.o \
		  Router.o \
		  SSLInfo.o \
		  Session.o \
		  Application.o \
		  Metrics.o \
		  SessionHelper.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/Metrics.ice \
		  $(SDIR)/PermissionsVerifierF.ice \
		  $(SDIR)/PermissionsVerifier.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Session.ice \
		  $(SDIR)/SSLInfo.ice

HDIR		= $(headerdir)/Glacier2
SDIR		= $(slicedir)/Glacier2

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(OPENSSL_FLAGS) -DGLACIER2_API_EXPORTS
SLICE2CPPFLAGS	:= --ice --include-dir Glacier2 --dll-export GLACIER2_API $(SLICE2CPPFLAGS)
LINKWITH        := $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(libdir)/$(LIBFILENAME): $(OBJS) $(HDIR)/PermissionsVerifierF.h $(HDIR)/RouterF.h
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
