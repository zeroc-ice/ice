# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,Glacier2,$(VERSION))
SONAME		= $(call mksoname,Glacier2,$(SOVERSION))
LIBNAME		= $(call mklibname,Glacier2)

TARGETS 	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

SLICE_OBJS      = Metrics.o \
		  PermissionsVerifierF.o \
		  PermissionsVerifier.o \
		  Router.o \
		  RouterF.o \
		  Session.o \
		  SSLInfo.o

OBJS		= Application.o \
		  CryptPermissionsVerifierPlugin.o \
		  SessionHelper.o \
                  $(SLICE_OBJS)

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(addprefix $(SDIR)/, $(SLICE_OBJS:.o=.ice))

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
