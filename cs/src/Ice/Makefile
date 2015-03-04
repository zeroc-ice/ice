# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= Ice
LIBNAME		= $(PKG).dll
TARGETS		= $(assembliesdir)/$(LIBNAME)
POLICY_TARGET	= $(POLICY).dll

SRCS		= Acceptor.cs \
		  AMDCallback.cs \
		  Application.cs \
		  Arrays.cs \
		  AssemblyInfo.cs \
		  AssemblyUtil.cs \
		  AsyncIOThread.cs \
                  Base64.cs \
		  BasicStream.cs \
		  Buffer.cs \
		  ByteBuffer.cs \
		  CommunicatorI.cs \
                  Compare.cs \
		  CollectionBase.cs \
		  Collections.cs \
		  ConnectRequestHandler.cs \
		  ConnectionI.cs \
		  ConnectionFactory.cs \
		  ConnectionMonitor.cs \
		  ConnectionReaper.cs \
		  ConnectionRequestHandler.cs \
		  Connector.cs \
		  DefaultsAndOverrides.cs \
                  DictionaryBase.cs \
		  Direct.cs \
                  DispatchInterceptor.cs \
		  EndpointI.cs \
		  EndpointFactory.cs \
		  EndpointFactoryManager.cs \
		  EndpointHostResolver.cs \
		  EventHandler.cs \
		  Exception.cs \
		  FormatType.cs \
		  ImplicitContextI.cs \
		  IncomingAsync.cs \
		  Incoming.cs \
		  Instance.cs \
		  InstrumentationI.cs \
		  LocatorInfo.cs \
		  LoggerI.cs \
		  LoggerPlugin.cs \
		  MetricsAdminI.cs \
		  MetricsObserverI.cs \
		  Monitor.cs \
		  Network.cs \
		  ObjectAdapterFactory.cs \
		  ObjectAdapterI.cs \
		  Object.cs \
		  ObjectFactoryManager.cs \
		  ObserverHelper.cs \
		  OpaqueEndpointI.cs \
		  Optional.cs \
		  Options.cs \
		  OutgoingAsync.cs \
		  Outgoing.cs \
		  OutputBase.cs \
		  Patcher.cs \
		  PluginManagerI.cs \
                  ProcessI.cs \
                  PropertiesAdminI.cs \
		  PropertiesI.cs \
		  Property.cs \
		  PropertyNames.cs \
		  Protocol.cs \
		  ProtocolPluginFacade.cs \
		  Proxy.cs \
		  ProxyFactory.cs \
		  ProxyIdentityKey.cs \
		  Reference.cs \
		  ReferenceFactory.cs \
                  ReplyStatus.cs \
                  RequestHandler.cs \
		  RetryQueue.cs \
		  RouterInfo.cs \
		  ServantManager.cs \
		  SliceChecksums.cs \
		  SlicedData.cs \
		  SocketOperation.cs \
		  Stream.cs \
		  StreamI.cs \
                  StreamWrapper.cs \
		  StringUtil.cs \
		  SysLoggerI.cs \
		  TcpAcceptor.cs \
		  TcpConnector.cs \
		  TcpEndpointI.cs \
		  TcpTransceiver.cs \
		  ThreadHookPlugin.cs \
		  ThreadPool.cs \
		  TieBase.cs \
		  Time.cs \
		  Timer.cs \
		  TraceLevels.cs \
		  TraceUtil.cs \
		  Transceiver.cs \
		  UdpEndpointI.cs \
		  UdpConnector.cs \
		  UdpTransceiver.cs \
		  UnknownSlicedObject.cs \
		  UserExceptionFactory.cs \
		  Util.cs \
		  ValueWriter.cs

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/Connection.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/Endpoint.ice \
		  $(SDIR)/EndpointTypes.ice \
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/ImplicitContext.ice \
		  $(SDIR)/LocalException.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/Metrics.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/Instrumentation.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/Process.ice \
		  $(SDIR)/PropertiesAdmin.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/ServantLocator.ice \
		  $(SDIR)/SliceChecksumDict.ice \
		  $(SDIR)/Stats.ice \
		  $(SDIR)/Version.ice

SDIR		= $(slicedir)/Ice
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	:= $(MCSFLAGS) -keyfile:$(KEYFILE)
MCSFLAGS	:= $(MCSFLAGS) /doc:$(assembliesdir)/$(PKG).xml /nowarn:1591

ifeq ($(MANAGED),yes)
    MCSFLAGS	:= $(MCSFLAGS) -define:MANAGED
else
    MCSFLAGS    := $(MCSFLAGS) -unsafe+
endif

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(subst /,$(DSEP),$^)
	$(INSTALL) $(LIBNAME).config $(assembliesdir)

clean::
	-rm -f $(assembliesdir)/$(LIBNAME).config

ifeq ($(GACINSTALL),yes)
install:: all
	$(call installdata,../../lib/pkgconfig/$(PKG).pc,$(DESTDIR)$(install_pkgconfigdir))
else
install:: all
	$(call installdata,$(TARGETS).config,$(DESTDIR)$(install_assembliesdir))
endif

install:: all
	(cd $(assembliesdir); $(call installassembly,$(LIBNAME),$(PKG)); $(call installpolicy,$(POLICY)); \
	 $(call installmdb,$(LIBNAME).mdb); \
	 $(call installdata,$(PKG).xml,$(DESTDIR)$(install_assembliesdir)))

clean::
	-rm -f $(assembliesdir)/$(PKG).xml

$(GDIR)/BuiltinSequences.cs: $(SDIR)/BuiltinSequences.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	rm -f $(GDIR)/BuiltinSequences.cs
	$(SLICE2CS) $(SLICE2CSFLAGS) --stream $(SDIR)/BuiltinSequences.ice
	mv BuiltinSequences.cs $(GDIR)

include .depend
