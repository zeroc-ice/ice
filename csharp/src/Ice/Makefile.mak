# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= Ice
LIBNAME		= $(PKG).dll
TARGETS		= $(assembliesdir)\$(LIBNAME)
POLICY_TARGET   = $(POLICY).dll

SRCS		= Acceptor.cs \
		  ACM.cs \
		  AMDCallback.cs \
		  Application.cs \
		  Arrays.cs \
		  AssemblyInfo.cs \
		  AssemblyUtil.cs \
		  AsyncIOThread.cs \
		  AsyncResult.cs \
		  Base64.cs \
		  BasicStream.cs \
		  BatchRequestInterceptor.cs \
		  BatchRequestQueue.cs \
		  Buffer.cs \
		  ByteBuffer.cs \
		  CollectionBase.cs \
		  Collections.cs \
		  CollocatedRequestHandler.cs \
		  CommunicatorI.cs \
		  Compare.cs \
		  ConnectionFactory.cs \
		  ConnectionI.cs \
		  ConnectionRequestHandler.cs \
		  Connector.cs \
		  ConnectRequestHandler.cs \
		  DefaultsAndOverrides.cs \
		  DictionaryBase.cs \
		  DispatchInterceptor.cs \
		  EndpointFactory.cs \
		  EndpointFactoryManager.cs \
		  EndpointHostResolver.cs \
		  EndpointI.cs \
		  EventHandler.cs \
		  Exception.cs \
		  FormatType.cs \
		  HashSet.cs \
		  HttpParser.cs \
		  ImplicitContextI.cs \
		  Incoming.cs \
		  IncomingAsync.cs \
		  Instance.cs \
		  InstrumentationI.cs \
		  IPEndpointI.cs \
		  LocatorInfo.cs \
                  LoggerAdminI.cs \
                  LoggerAdminLoggerI.cs \
		  LoggerI.cs \
		  LoggerPlugin.cs \
		  MetricsAdminI.cs \
		  MetricsObserverI.cs \
		  Network.cs \
		  NetworkProxy.cs \
		  Object.cs \
		  ObjectAdapterFactory.cs \
		  ObjectAdapterI.cs \
		  ObjectFactoryManager.cs \
		  ObserverHelper.cs \
		  OpaqueEndpointI.cs \
		  Optional.cs \
		  Options.cs \
		  OutgoingAsync.cs \
		  OutputBase.cs \
		  Patcher.cs \
		  PluginManagerI.cs \
		  ProcessI.cs \
		  PropertiesAdminI.cs \
		  PropertiesI.cs \
		  Property.cs \
		  PropertyNames.cs \
		  Protocol.cs \
		  ProtocolInstance.cs \
		  ProtocolPluginFacade.cs \
		  Proxy.cs \
		  ProxyFactory.cs \
		  ProxyIdentityKey.cs \
		  Reference.cs \
		  ReferenceFactory.cs \
		  ReplyStatus.cs \
		  RequestHandler.cs \
		  RequestHandlerFactory.cs \
		  ResponseHandler.cs \
		  RetryQueue.cs \
		  RouterInfo.cs \
		  ServantManager.cs \
		  SliceChecksums.cs \
		  SlicedData.cs \
		  SocketOperation.cs \
		  Stream.cs \
		  StreamI.cs \
		  StreamSocket.cs \
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
		  UdpConnector.cs \
		  UdpEndpointI.cs \
		  UdpTransceiver.cs \
		  UnknownSlicedObject.cs \
		  UserExceptionFactory.cs \
		  Util.cs \
		  ValueWriter.cs \
		  WSAcceptor.cs \
		  WSConnector.cs \
		  WSEndpoint.cs \
		  WSTransceiver.cs

GEN_SRCS	= $(GDIR)\BuiltinSequences.cs \
		  $(GDIR)\Communicator.cs \
		  $(GDIR)\Connection.cs \
		  $(GDIR)\Current.cs \
		  $(GDIR)\Endpoint.cs \
		  $(GDIR)\EndpointTypes.cs \
		  $(GDIR)\FacetMap.cs \
		  $(GDIR)\Identity.cs \
		  $(GDIR)\ImplicitContext.cs \
		  $(GDIR)\Instrumentation.cs \
		  $(GDIR)\LocalException.cs \
		  $(GDIR)\Locator.cs \
		  $(GDIR)\Logger.cs \
		  $(GDIR)\Metrics.cs \
		  $(GDIR)\ObjectAdapter.cs \
		  $(GDIR)\ObjectFactory.cs \
		  $(GDIR)\Plugin.cs \
		  $(GDIR)\Process.cs \
		  $(GDIR)\Properties.cs \
		  $(GDIR)\PropertiesAdmin.cs \
                  $(GDIR)\RemoteLogger.cs \
		  $(GDIR)\Router.cs \
		  $(GDIR)\ServantLocator.cs \
		  $(GDIR)\SliceChecksumDict.cs \
		  $(GDIR)\Version.cs

SDIR		= $(slicedir)\Ice
GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	= $(MCSFLAGS) -keyfile:"$(KEYFILE)"
MCSFLAGS	= $(MCSFLAGS) /doc:$(assembliesdir)\$(PKG).xml /nowarn:1591

!if "$(MANAGED)" != "yes"
MCSFLAGS        = $(MCSFLAGS) /unsafe
!endif

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --ice -I"$(slicedir)"

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) /baseaddress:0x20000000 $(MCSFLAGS) $(SRCS) $(GEN_SRCS)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

!if "$(DEBUG)" == "yes"
clean::
	del /q $(assembliesdir)\$(PKG).pdb
!endif

clean::
	del /q $(assembliesdir)\$(PKG).xml

install:: all
	copy $(assembliesdir)\$(LIBNAME) "$(install_assembliesdir)"
	copy $(assembliesdir)\$(PKG).xml "$(install_assembliesdir)"
!if "$(generate_policies)" == "yes"
	copy $(assembliesdir)\$(POLICY_TARGET) "$(install_assembliesdir)"
!endif
!if "$(DEBUG)" == "yes"
	copy $(assembliesdir)\$(PKG).pdb "$(install_assembliesdir)"
!endif

$(GDIR)\BuiltinSequences.cs: "$(SDIR)\BuiltinSequences.ice" "$(SLICE2CS)" "$(SLICEPARSERLIB)"
	del /q $(GDIR)\BuiltinSequences.cs
	"$(SLICE2CS)" $(SLICE2CSFLAGS) --stream "$(SDIR)\BuiltinSequences.ice"
	move BuiltinSequences.cs $(GDIR)
