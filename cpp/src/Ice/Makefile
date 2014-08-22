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
		  ACM.o \
		  Application.o \
		  Base64.o \
		  BasicStream.o \
		  Buffer.o \
		  BuiltinSequences.o \
		  CollocatedRequestHandler.o \
		  Communicator.o \
		  CommunicatorI.o \
		  Connection.o \
		  ConnectionFactory.o \
		  ConnectionI.o \
		  ConnectionRequestHandler.o \
		  Connector.o \
		  ConnectRequestHandler.o \
		  Current.o \
		  DefaultsAndOverrides.o \
		  DeprecatedStringConverter.o \
		  DispatchInterceptor.o \
		  DynamicLibrary.o \
		  Endpoint.o \
		  EndpointFactory.o \
		  EndpointFactoryManager.o \
		  EndpointI.o \
		  EndpointTypes.o \
		  EventHandler.o \
		  Exception.o \
		  FacetMap.o \
		  FactoryTable.o \
		  FactoryTableInit.o \
		  GCObject.o \
		  HttpParser.o \
		  Identity.o \
		  ImplicitContext.o \
		  ImplicitContextI.o \
		  Incoming.o \
		  IncomingAsync.o \
		  Initialize.o \
		  Instance.o \
		  Instrumentation.o \
		  InstrumentationI.o \
		  IPEndpointI.o \
		  LocalException.o \
		  LocalObject.o \
		  Locator.o \
		  LocatorInfo.o \
		  Logger.o \
		  LoggerAdminI.o \
		  LoggerI.o \
		  LoggerUtil.o \
		  Metrics.o \
		  MetricsAdminI.o \
		  MetricsObserverI.o \
		  Network.o \
		  Object.o \
		  ObjectAdapter.o \
		  ObjectAdapterFactory.o \
		  ObjectAdapterI.o \
		  ObjectFactory.o \
		  ObjectFactoryManager.o \
		  ObserverHelper.o \
		  OpaqueEndpointI.o \
		  Outgoing.o \
		  OutgoingAsync.o \
		  Plugin.o \
		  PluginManagerI.o \
		  Process.o \
		  Properties.o \
		  PropertiesAdmin.o \
		  PropertiesAdminI.o \
		  PropertiesI.o \
		  PropertyNames.o \
		  Protocol.o \
		  ProtocolInstance.o \
		  ProtocolPluginFacade.o \
		  Proxy.o \
		  ProxyFactory.o \
		  Reference.o \
		  ReferenceFactory.o \
		  RemoteLogger.o \
		  RequestHandler.o \
		  ResponseHandler.o \
		  RetryQueue.o \
		  Router.o \
		  RouterInfo.o \
		  Selector.o \
		  ServantLocator.o \
		  ServantManager.o \
		  Service.o \
		  SliceChecksumDict.o \
		  SliceChecksums.o \
		  SlicedData.o \
		  Stream.o \
		  StreamI.o \
		  StringConverterPlugin.o \
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
		  Version.o \
		  WSAcceptor.o \
		  WSConnector.o \
		  WSEndpoint.o \
		  WSTransceiver.o

ifeq ($(findstring MINGW,$(shell uname)),)
OBJS		:= $(OBJS) \
		  SysLoggerI.o
endif

SRCS		= $(OBJS:.o=.cpp) \
		  DLLMain.cpp

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/CommunicatorF.ice \
		  $(SDIR)/Connection.ice \
		  $(SDIR)/ConnectionF.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/Endpoint.ice \
		  $(SDIR)/EndpointF.ice \
		  $(SDIR)/EndpointTypes.ice \
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/ImplicitContext.ice \
		  $(SDIR)/ImplicitContextF.ice \
		  $(SDIR)/Instrumentation.ice \
		  $(SDIR)/InstrumentationF.ice \
		  $(SDIR)/LocalException.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LocatorF.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/LoggerF.ice \
		  $(SDIR)/Metrics.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/ObjectAdapterF.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/ObjectFactoryF.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/PluginF.ice \
		  $(SDIR)/Process.ice \
		  $(SDIR)/ProcessF.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/PropertiesAdmin.ice \
		  $(SDIR)/PropertiesF.ice \
		  $(SDIR)/RemoteLogger.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/ServantLocator.ice \
		  $(SDIR)/ServantLocatorF.ice \
		  $(SDIR)/SliceChecksumDict.ice \
		  $(SDIR)/Version.ice

HDIR		= $(headerdir)/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_API_EXPORTS $(BZIP2_FLAGS)
ifneq ($(COMPSUFFIX),)
CPPFLAGS	:= $(CPPFLAGS) -DCOMPSUFFIX=\"$(COMPSUFFIX)\"
endif
SLICE2CPPFLAGS	:= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)

LINKWITH        := -lIceUtil $(BZIP2_LIBS) $(ICE_OS_LIBS) $(ICONV_LIBS)

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
