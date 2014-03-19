# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,Ice,$(VERSION))
SONAME		= $(call mksoname,Ice,$(SOVERSION))  
LIBNAME		= $(call mklibname,Ice)
TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= Acceptor.o \
		  Application.o \
                  Base64.o \
		  Buffer.o \
		  BasicStream.o \
		  BuiltinSequences.o \
		  CommunicatorI.o \
		  Communicator.o \
		  ConnectRequestHandler.o \
		  ConnectionFactory.o \
		  ConnectionI.o \
		  ConnectionMonitor.o \
		  Connection.o \
		  ConnectionRequestHandler.o \
		  Connector.o \
		  Current.o \
		  DefaultsAndOverrides.o \
		  Direct.o \
                  DispatchInterceptor.o \
		  DynamicLibrary.o \
		  Endpoint.o \
		  EndpointFactoryManager.o \
		  EndpointFactory.o \
		  EndpointI.o \
		  EndpointTypes.o \
		  EventHandler.o \
		  Exception.o \
		  FacetMap.o \
		  FactoryTable.o \
		  FactoryTableInit.o \
		  GC.o \
		  Identity.o \
		  ImplicitContextI.o \
		  ImplicitContext.o \
		  IncomingAsync.o \
		  Incoming.o \
		  Initialize.o \
		  Instance.o \
		  IPEndpointI.o \
		  LocalException.o \
		  LocalObject.o \
		  LocatorInfo.o \
		  Locator.o \
		  LoggerI.o \
		  Logger.o \
		  LoggerUtil.o \
		  Metrics.o \
		  MetricsAdminI.o \
		  MetricsObserverI.o \
		  Network.o \
		  ObjectAdapterFactory.o \
		  ObjectAdapterI.o \
		  ObjectAdapter.o \
		  ObjectFactoryManager.o \
		  ObjectFactory.o \
		  Object.o \
		  Instrumentation.o \
		  ObserverHelper.o \
		  InstrumentationI.o \
		  OpaqueEndpointI.o \
		  OutgoingAsync.o \
		  Outgoing.o \
		  PluginManagerI.o \
		  Plugin.o \
		  Process.o \
		  PropertiesAdminI.o \
		  PropertiesI.o \
		  PropertiesAdmin.o \
		  Properties.o \
		  PropertyNames.o \
		  Protocol.o \
		  ProtocolInstance.o \
		  ProtocolPluginFacade.o \
		  ProxyFactory.o \
		  Proxy.o \
		  ReferenceFactory.o \
		  Reference.o \
		  RetryQueue.o \
		  RequestHandler.o \
		  RouterInfo.o \
		  Router.o \
		  Selector.o \
		  ServantLocator.o \
		  ServantManager.o \
		  Service.o \
		  SliceChecksumDict.o \
		  SliceChecksums.o \
		  SlicedData.o \
		  Stats.o \
		  StreamI.o \
		  Stream.o \
		  StringConverter.o \
		  TcpAcceptor.o \
		  TcpConnector.o \
		  TcpEndpointI.o \
		  TcpTransceiver.o \
	          ThreadPool.o \
		  TraceLevels.o \
		  TraceUtil.o \
		  Transceiver.o \
		  UdpConnector.o \
		  UdpEndpointI.o \
		  UdpTransceiver.o \
		  Version.o

ifeq ($(findstring MINGW,$(shell uname)),)
OBJS		:= $(OBJS) \
		  SysLoggerI.o
endif

SRCS		= $(OBJS:.o=.cpp) \
		  DLLMain.cpp

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/CommunicatorF.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/ConnectionF.ice \
		  $(SDIR)/Connection.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/Endpoint.ice \
		  $(SDIR)/EndpointF.ice \
		  $(SDIR)/EndpointTypes.ice \
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/ImplicitContextF.ice \
		  $(SDIR)/ImplicitContext.ice \
		  $(SDIR)/LocalException.ice \
		  $(SDIR)/LocatorF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LoggerF.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/Metrics.ice \
		  $(SDIR)/ObjectAdapterF.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/ObjectFactoryF.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/Instrumentation.ice \
		  $(SDIR)/InstrumentationF.ice \
		  $(SDIR)/PluginF.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/ProcessF.ice \
		  $(SDIR)/Process.ice \
		  $(SDIR)/PropertiesF.ice \
		  $(SDIR)/PropertiesAdmin.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/ServantLocatorF.ice \
		  $(SDIR)/ServantLocator.ice \
		  $(SDIR)/SliceChecksumDict.ice \
		  $(SDIR)/StatsF.ice \
		  $(SDIR)/Stats.ice \
		  $(SDIR)/Version.ice \

HDIR		= $(headerdir)/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_API_EXPORTS $(BZIP2_FLAGS)
ifneq ($(COMPSUFFIX),)
CPPFLAGS	:= $(CPPFLAGS) -DCOMPSUFFIX=\"$(COMPSUFFIX)\"
endif
SLICE2CPPFLAGS	:= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)

LINKWITH        := -lIceUtil $(BZIP2_LIBS) $(ICONV_LIBS) $(ICE_OS_LIBS)

ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@
endif

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
