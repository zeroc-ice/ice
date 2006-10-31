# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= icecs
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\$(LIBNAME)

SRCS		= Acceptor.cs \
		  Application.cs \
		  Arrays.cs \
		  AssemblyInfo.cs \
		  AssemblyUtil.cs \
		  Assert.cs \
		  BasicInputStream.cs \
		  BasicOutputStream.cs \
		  BasicStream.cs \
		  ByteBuffer.cs \
		  CommunicatorI.cs \
		  ConnectionI.cs \
		  ConnectionFactory.cs \
		  ConnectionMonitor.cs \
		  Connector.cs \
		  DefaultsAndOverrides.cs \
		  Direct.cs \
		  EndpointI.cs \
		  EndpointFactory.cs \
		  EndpointFactoryManager.cs \
		  EventHandler.cs \
		  Exception.cs \
		  ImplicitContextI.cs \
		  IncomingAsync.cs \
		  Incoming.cs \
		  InputStream.cs \
		  InputStreamI.cs \
		  Instance.cs \
		  LinkedList.cs \
		  LocalObject.cs \
		  LocatorInfo.cs \
		  LoggerI.cs \
		  Network.cs \
		  ObjectAdapterFactory.cs \
		  ObjectAdapterI.cs \
		  Object.cs \
		  ObjectFactoryManager.cs \
		  ObjectReader.cs \
		  ObjectWriter.cs \
		  Options.cs \
		  OutgoingAsync.cs \
		  Outgoing.cs \
		  OutputBase.cs \
		  OutputStream.cs \
		  OutputStreamI.cs \
		  Patcher.cs \
		  PluginManagerI.cs \
		  PropertiesI.cs \
		  PropertyNames.cs \
		  Protocol.cs \
		  ProtocolPluginFacade.cs \
		  Proxy.cs \
		  ProxyFactory.cs \
		  ProxyIdentityKey.cs \
		  ReadObjectCallback.cs \
		  Reference.cs \
		  ReferenceFactory.cs \
		  RouterInfo.cs \
		  ServantManager.cs \
		  Set.cs \
		  SliceChecksums.cs \
		  StringUtil.cs \
		  SysLoggerI.cs \
		  TcpAcceptor.cs \
		  TcpConnector.cs \
		  TcpEndpointI.cs \
		  TcpTransceiver.cs \
		  ThreadPool.cs \
		  TieBase.cs \
		  TraceLevels.cs \
		  TraceUtil.cs \
		  Transceiver.cs \
		  UdpEndpointI.cs \
		  UdpTransceiver.cs \
		  UnknownEndpointI.cs \
		  UserExceptionFactory.cs \
		  Util.cs \
		  ValueWriter.cs \
		  XMLOutput.cs

GEN_SRCS	= $(GDIR)\BuiltinSequences.cs \
		  $(GDIR)\CommunicatorF.cs \
		  $(GDIR)\Communicator.cs \
		  $(GDIR)\Connection.cs \
		  $(GDIR)\ConnectionF.cs \
		  $(GDIR)\Current.cs \
		  $(GDIR)\Endpoint.cs \
		  $(GDIR)\FacetMap.cs \
		  $(GDIR)\Identity.cs \
		  $(GDIR)\ImplicitContext.cs \
		  $(GDIR)\LocalException.cs \
		  $(GDIR)\LocatorF.cs \
		  $(GDIR)\Locator.cs \
		  $(GDIR)\LoggerF.cs \
		  $(GDIR)\Logger.cs \
		  $(GDIR)\ObjectAdapterF.cs \
		  $(GDIR)\ObjectAdapter.cs \
		  $(GDIR)\ObjectFactoryF.cs \
		  $(GDIR)\ObjectFactory.cs \
		  $(GDIR)\PluginF.cs \
		  $(GDIR)\Plugin.cs \
		  $(GDIR)\ProcessF.cs \
		  $(GDIR)\Process.cs \
		  $(GDIR)\PropertiesF.cs \
		  $(GDIR)\Properties.cs \
		  $(GDIR)\RouterF.cs \
		  $(GDIR)\Router.cs \
		  $(GDIR)\ServantLocatorF.cs \
		  $(GDIR)\ServantLocator.cs \
		  $(GDIR)\SliceChecksumDict.cs \
		  $(GDIR)\StatsF.cs \
		  $(GDIR)\Stats.cs

SDIR		= $(slicedir)\Ice
GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe -warnaserror-

!if "$(DOTNET_1)" != "yes"
MCSFLAGS	= $(MCSFLAGS) -keyfile:$(top_srcdir)\config\IcecsKey.snk
!endif

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(SRCS) $(GEN_SRCS)

!if "$(NOGAC)" == ""

install:: all
	$(GACUTIL) -i $(bindir)\$(LIBNAME)

!else

install:: all
	copy $(bindir)\icecs.dll $(install_bindir)

!endif

!include .depend
