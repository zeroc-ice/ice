# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,Glacier2ObjC,$(VERSION))
SONAME		= $(call mksoname,Glacier2ObjC,$(SOVERSION))
LIBNAME		= $(call mklibname,Glacier2ObjC)

TARGETS 	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

SLICE_OBJS  	= Metrics.o \
		  PermissionsVerifierF.o \
		  PermissionsVerifier.o \
		  Router.o \
		  RouterF.o \
		  Session.o \
		  SSLInfo.o

OBJS		= $(SLICE_OBJS)

HDIR		= $(headerdir)/objc/Glacier2
SDIR		= $(slicedir)/Glacier2

include $(top_srcdir)/config/Make.rules

SLICE2OBJCFLAGS	:= --ice --include-dir objc/Glacier2 --dll-export GLACIER2_API $(SLICE2OBJCFLAGS)

$(libdir)/$(LIBFILENAME): $(OBJS) $(HDIR)/PermissionsVerifierF.h $(HDIR)/RouterF.h
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
