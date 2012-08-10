# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= Ice
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\$(LIBNAME)
POLICY_TARGET   = $(POLICY).dll

SRCS		= Acceptor.cs \
		  Application.cs \
		  AMDCallback.cs \
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
		  HashSet.cs \
		  ImplicitContextI.cs \
		  IncomingAsync.cs \
		  Incoming.cs \
		  Instance.cs \
		  LocatorInfo.cs \
		  LoggerI.cs \
		  LoggerPlugin.cs \
		  Monitor.cs \
		  Network.cs \
		  ObjectAdapterFactory.cs \
		  ObjectAdapterI.cs \
		  Object.cs \
		  ObjectFactoryManager.cs \
		  OPaqueEndpointI.cs \
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
		  SocketOperation.cs \
		  SliceChecksums.cs \
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
		  UdpConnector.cs \
		  UdpEndpointI.cs \
		  UdpTransceiver.cs \
		  UserExceptionFactory.cs \
		  Util.cs \
		  ValueWriter.cs

GEN_SRCS	= $(GDIR)\BuiltinSequences.cs \
		  $(GDIR)\Communicator.cs \
		  $(GDIR)\Connection.cs \
		  $(GDIR)\Current.cs \
		  $(GDIR)\Endpoint.cs \
		  $(GDIR)\EndpointTypes.cs \
		  $(GDIR)\FacetMap.cs \
		  $(GDIR)\Identity.cs \
		  $(GDIR)\ImplicitContext.cs \
		  $(GDIR)\LocalException.cs \
		  $(GDIR)\Locator.cs \
		  $(GDIR)\Logger.cs \
		  $(GDIR)\ObjectAdapter.cs \
		  $(GDIR)\ObjectFactory.cs \
		  $(GDIR)\Plugin.cs \
		  $(GDIR)\Process.cs \
		  $(GDIR)\Properties.cs \
		  $(GDIR)\Router.cs \
		  $(GDIR)\ServantLocator.cs \
		  $(GDIR)\SliceChecksumDict.cs \
		  $(GDIR)\Stats.cs \
		  $(GDIR)\Version.cs

SDIR		= $(slicedir)\Ice
GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	= $(MCSFLAGS) -keyfile:$(KEYFILE)
MCSFLAGS	= $(MCSFLAGS) /doc:$(bindir)\$(PKG).xml /nowarn:1591

!if "$(MANAGED)" == "yes"
MCSFLAGS	= $(MCSFLAGS) -define:MANAGED
!else
MCSFLAGS        = $(MCSFLAGS) /unsafe
!endif

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) /baseaddress:0x20000000 $(MCSFLAGS) $(SRCS) $(GEN_SRCS)

!if "$(DEBUG)" == "yes"
clean::
	del /q $(bindir)\$(PKG).pdb
!endif

clean::
	del /q $(bindir)\$(PKG).xml

install:: all
	copy $(bindir)\$(LIBNAME) "$(install_bindir)"
	copy $(bindir)\$(PKG).xml "$(install_bindir)"
!if "$(generate_policies)" == "yes"
	copy $(bindir)\$(POLICY) "$(install_bindir)"
	copy $(bindir)\$(POLICY_TARGET) "$(install_bindir)"
!endif
!if "$(DEBUG)" == "yes"
	copy $(bindir)\$(PKG).pdb "$(install_bindir)"
!endif

$(GDIR)\BuiltinSequences.cs: $(SDIR)\BuiltinSequences.ice $(SLICE2CS) $(SLICEPARSERLIB)
	del /q $(GDIR)\BuiltinSequences.cs
	$(SLICE2CS) $(SLICE2CSFLAGS) --stream $(SDIR)\BuiltinSequences.ice
	move BuiltinSequences.cs $(GDIR)

!include .depend.mak
