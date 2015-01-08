# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Touch is licensed to you under the terms described in the
# ICE_TOUCH_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME = $(call mklibfilename,IceObjC$(libsuffix),$(VERSION))
SONAME		= $(call mksoname,IceObjC$(libsuffix),$(SOVERSION))
LIBNAME		= $(call mklibname,IceObjC$(libsuffix))

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

SLICE_OBJS	= BuiltinSequences.o \
			  Identity.o \
			  Locator.o \
			  LocatorF.o \
			  LocalException.o \
			  Metrics.o \
			  Process.o \
			  ProcessF.o \
			  PropertiesAdmin.o \
			  RemoteLogger.o \
			  Router.o \
			  RouterF.o \
			  SliceChecksumDict.o \
			  Version.o

OBJC_OBJS	= DispatchInterceptor.o \
		  	  Request.o \
			  $(SLICE_OBJS)

OBJCXX_OBJS	= Communicator.o \
			  Connection.o \
			  Current.o \
			  Exception.o \
			  Dispatcher.o \
			  IdentityI.o \
			  Initialize.o \
			  Logger.o \
			  Object.o \
			  ObjectAdapter.o \
			  Properties.o \
			  Proxy.o \
			  Stream.o \
			  SlicedData.o \
			  Util.o \
			  VersionI.o \
			  Wrapper.o \
			  ImplicitContext.o \
			  Endpoint.o

OBJS		:= $(OBJC_OBJS) $(OBJCXX_OBJS)

HDIR		= $(includedir)/objc/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS		:= -I. -Iinclude $(CPPFLAGS)
SLICE2OBJCFLAGS	:= --ice --include-dir objc/Ice $(SLICE2OBJCFLAGS)
DEPENDFLAGS		= --obj-dir
LINKWITH		:= $(BASELIBS)

ifeq ($(wildcard .depend/*.d),)

ice_cpp_include = $(ice_cpp_dir)/include

$(OBJCXX_OBJS:.o=.mm): \
	$(patsubst $(ice_cpp_include)/IceUtil/%.h,include/IceUtilCpp/%.h,$(wildcard $(ice_cpp_include)/IceUtil/*.h)) \
	$(patsubst $(ice_cpp_include)/Ice/%.h,include/IceCpp/%.h,$(wildcard $(ice_cpp_include)/Ice/*.h)) \
	$(patsubst $(ice_cpp_include)/IceSSL/%.h,include/IceSSLCpp/%.h,$(wildcard $(ice_cpp_include)/IceSSL/*.h))

include/IceUtilCpp/%.h: $(ice_cpp_include)/IceUtil/%.h
	@echo Creating $@ from $<
	@mkdir -p include/IceUtilCpp
	@sed -e "s%IceUtil/%IceUtilCpp/%g" $< > $@

include/IceCpp/%.h:  $(ice_cpp_include)/Ice/%.h
	@echo Creating $@ from $<
	@mkdir -p include/IceCpp
	@sed -e "s%IceUtil/%IceUtilCpp/%g;s%Ice/%IceCpp/%g" $< > $@

include/IceSSLCpp/%.h: $(ice_cpp_include)/IceSSL/%.h
	@echo Creating $@ from $<
	@mkdir -p include/IceSSLCpp
	@sed -e "s%IceUtil/%IceUtilCpp/%g;s%Ice/%IceCpp/%g;s%IceSSL/%IceSSLCpp/%g" $< > $@

endif

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

# Prevent generation of these files from .ice files

$(HDIR)/ImplicitContext.h ImplicitContext.m:
	@echo

$(HDIR)/Communicator.h Communicator.m:
	@echo

$(HDIR)/Current.h Current.m:
	@echo

$(HDIR)/Logger.h Logger.m:
	@echo

$(HDIR)/ObjectAdapter.h ObjectAdapter.m:
	@echo

$(HDIR)/Properties.h Properties.m:
	@echo

$(HDIR)/Endpoint.h Endpoint.m:
	@echo

$(HDIR)/Connection.h Connection.m:
	@echo

$(HDIR)/ObjectFactory.h ObjectFactory.m:
	@echo

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

clean::
	rm -rf include
