# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,Ice,$(VERSION))
SONAME		= $(call mksoname,Ice,$(SOVERSION))
LIBNAME		= $(call mklibname,Ice)
TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

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
		  ImplicitContext.o \
		  ImplicitContextF.o \
		  Instrumentation.o \
		  InstrumentationF.o \
		  LocalException.o \
		  Locator.o \
		  LocatorF.o \
		  Logger.o \
		  LoggerF.o \
		  Metrics.o \
		  ObjectAdapter.o \
		  ObjectAdapterF.o \
		  ObjectFactory.o \
		  ObjectFactoryF.o \
		  Plugin.o \
		  PluginF.o \
		  Process.o \
		  ProcessF.o \
		  Properties.o \
		  PropertiesAdmin.o \
		  PropertiesF.o \
		  RemoteLogger.o \
		  Router.o \
		  RouterF.o \
		  ServantLocator.o \
		  ServantLocatorF.o \
		  SliceChecksumDict.o \
		  Version.o

OBJS		= Acceptor.o \
		  ACM.o \
		  Application.o \
	 	  AsyncResult.o \
		  Base64.o \
		  BasicStream.o \
		  BatchRequestQueue.o \
		  Buffer.o \
		  CollocatedRequestHandler.o \
		  CommunicatorI.o \
		  ConnectionFactory.o \
		  ConnectionI.o \
		  ConnectionRequestHandler.o \
		  Connector.o \
		  ConnectRequestHandler.o \
		  DefaultsAndOverrides.o \
		  DeprecatedStringConverter.o \
		  DispatchInterceptor.o \
		  DynamicLibrary.o \
		  EndpointFactory.o \
		  EndpointFactoryManager.o \
		  EndpointI.o \
		  EventHandler.o \
		  Exception.o \
		  FactoryTable.o \
		  FactoryTableInit.o \
		  GCObject.o \
		  HttpParser.o \
		  ImplicitContextI.o \
		  Incoming.o \
		  IncomingAsync.o \
		  Initialize.o \
		  Instance.o \
		  InstrumentationI.o \
		  IPEndpointI.o \
		  LocalObject.o \
		  LocatorInfo.o \
		  LoggerAdminI.o \
		  LoggerI.o \
		  LoggerUtil.o \
		  MetricsAdminI.o \
		  MetricsObserverI.o \
		  Network.o \
		  NetworkProxy.o \
		  Object.o \
		  ObjectAdapterFactory.o \
		  ObjectAdapterI.o \
		  ObjectFactoryManager.o \
		  ObserverHelper.o \
		  OpaqueEndpointI.o \
		  Outgoing.o \
		  OutgoingAsync.o \
		  PluginManagerI.o \
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
		  RegisterPluginsInit.o \
		  RequestHandler.o \
		  RequestHandlerFactory.o \
		  ResponseHandler.o \
		  RetryQueue.o \
		  RouterInfo.o \
		  Selector.o \
		  ServantManager.o \
		  SliceChecksums.o \
		  SlicedData.o \
		  Stream.o \
		  StreamI.o \
		  StreamSocket.o \
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
		  WSAcceptor.o \
		  WSConnector.o \
		  WSEndpoint.o \
		  WSTransceiver.o \
		  $(SLICE_OBJS)

ifeq ($(findstring MINGW,$(shell uname)),)
OBJS		:= $(OBJS) \
		  Service.o \
		  SysLoggerI.o
endif

HDIR		= $(headerdir)/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_API_EXPORTS $(BZIP2_FLAGS)
ifneq ($(COMPSUFFIX),)
CPPFLAGS	:= $(CPPFLAGS) -DCOMPSUFFIX=\"$(COMPSUFFIX)\"
endif
SLICE2CPPFLAGS	:= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)

LINKWITH        := -lIceUtil $(BZIP2_LIBS) $(ICE_OS_LIBS) $(ICONV_LIBS)

ifeq ($(UNAME),MINGW)
$(OBJS): $(BZIP2_NUPKG)
endif

ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)$(cpp11libdirsuffix)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)$(cpp11libdirsuffix)
	rm -f $@
	ln -s $(cpp11sonamedir)$(SONAME) $@
endif

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
ifeq ($(UNAME),MINGW)
	$(call installlib,$(DESTDIR)$(install_libdir),$(BZIP2_HOME)/build/native/bin/$(PLATFORM),bzip2_mingw.dll,,)
endif