# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME = $(call mklibfilename,IceObjC$(libsuffix),$(VERSION))
SONAME		= $(call mksoname,IceObjC$(libsuffix),$(SOVERSION))
LIBNAME		= $(call mklibname,IceObjC$(libsuffix))

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

SLICE_OBJS	= BuiltinSequences.o \
		  Communicator.o \
		  CommunicatorF.o \
		  Connection.o \
		  ConnectionF.o \
		  Current.o \
		  Endpoint.o \
		  EndpointF.o \
		  EndpointTypes.o \
		  FacetMap.o \
		  Identity.o \
		  InstrumentationF.o \
		  ImplicitContext.o \
		  ImplicitContextF.o \
		  Locator.o \
		  LocatorF.o \
		  LocalException.o \
		  Logger.o \
		  LoggerF.o \
		  Metrics.o \
		  ObjectAdapter.o \
		  ObjectAdapterF.o \
		  ObjectFactoryF.o \
		  PluginF.o \
		  Process.o \
		  ProcessF.o \
		  Properties.o \
		  PropertiesF.o \
		  PropertiesAdmin.o \
		  RemoteLogger.o \
		  Router.o \
		  RouterF.o \
	  	  ServantLocator.o \
		  ServantLocatorF.o \
		  SliceChecksumDict.o \
		  Version.o

OBJC_OBJS	= DispatchInterceptor.o \
		  Request.o \
		  $(SLICE_OBJS)

OBJCXX_OBJS	= BatchRequestInterceptor.o \
		  CommunicatorI.o \
		  ConnectionI.o \
		  CurrentI.o \
		  EndpointI.o \
		  Exception.o \
		  Dispatcher.o \
		  IdentityI.o \
		  ImplicitContextI.o \
		  Initialize.o \
		  LocalObject.o \
		  LoggerI.o \
		  Object.o \
		  ObjectAdapterI.o \
		  PropertiesI.o \
		  Proxy.o \
		  Stream.o \
		  SlicedData.o \
		  Util.o \
		  VersionI.o

OBJS		:= $(OBJC_OBJS) $(OBJCXX_OBJS)

HDIR		= $(includedir)/objc/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

#
# Set RPATH to the cpp/lib directory or Ice installation lib directory. Note that unlike Linux
# platforms, we always set the RPATH for source builds on OS X. This is required because we
# can no longer rely on DYLD_LIBRARY_PATH on El Capitan. The RPATH is updated when the library
# is installed (see cpp/config/Make.rules.Darwin).
#
ifdef ice_src_dist
    RPATH_DIR = @loader_path/../../cpp/$(libsubdir)
else
    RPATH_DIR = $(ice_dir)/$(libsubdir)
endif

ifdef usr_dir_install
    installlib += ; install_name_tool -delete_rpath $(RPATH_DIR) $(1)/$(3)
else ifdef ice_src_dist
    installlib += ; install_name_tool -delete_rpath $(RPATH_DIR) $(1)/$(3)
endif

CPPFLAGS	:= -I. -I$(ice_cpp_dir)/include -DICE_API_EXPORTS $(CPPFLAGS)
SLICE2OBJCFLAGS	:= --ice --include-dir objc/Ice --dll-export ICE_API $(SLICE2OBJCFLAGS)
LINKWITH	:= $(BASELIBS)
LDFLAGS		:= -Wl,-rpath,$(RPATH_DIR) $(LDFLAGS)

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

$(HDIR)/ObjectFactory.h ObjectFactory.m:
	@echo

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
