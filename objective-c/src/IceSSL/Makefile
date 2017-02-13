# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,IceSSLObjC,$(VERSION))
SONAME		= $(call mksoname,IceSSLObjC,$(SOVERSION))
LIBNAME		= $(call mklibname,IceSSLObjC)

TARGETS 	= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

SLICE_OBJS	= EndpointInfo.o \
		  ConnectionInfo.o

OBJCXX_OBJS	= EndpointInfoI.o \
		  ConnectionInfoI.o

OBJS		= $(SLICE_OBJS) $(OBJCXX_OBJS)

HDIR		= $(headerdir)/objc/IceSSL
SDIR		= $(slicedir)/IceSSL

include $(top_srcdir)/config/Make.rules

CPPFLAGS		:= -I../Ice -I$(ice_cpp_dir)/include $(CPPFLAGS)
SLICE2OBJCFLAGS	:= --ice --include-dir objc/IceSSL --dll-export ICE_SSL_API $(SLICE2OBJCFLAGS)
LINKWITH        := -lIceObjC$(libsuffix) $(BASELIBS)

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
