# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME		= Icicle.dll
TARGETS		= $(libdir)/$(LIBNAME)

SRCS		= Acceptor.cs \
		  _AMD_Object_ice_invoke.cs \
		  AMD_Object_ice_invoke.cs \
		  AMI_Object_ice_invoke.cs \
		  Application.cs \
		  AssemblyInfo.cs \
		  BasicStream.cs \
		  BlobjectAsync.cs \
		  Blobject.cs \
		  BufferManager.cs \
		  ByteBuffer.cs \
		  CommunicatorI.cs \
		  Connection.cs \
		  ConnectionMonitor.cs \
		  Connector.cs \
		  DefaultsAndOverrides.cs \
		  Direct.cs \
		  DispatchStatus.cs \
		  Endpoint.cs \
		  EndpointFactory.cs \
		  EndpointFactoryManager.cs \
		  EventHandler.cs \
		  Exception.cs \
		  IncomingAsync.cs \
		  IncomingBase.cs \
		  IncomingConnectionFactory.cs \
		  Incoming.cs \
		  Instance.cs \
		  LinkedList.cs \
		  ListPatcher.cs \
		  LocalObject.cs \
		  LocalObjectImpl.cs \
		  LocatorInfo.cs \
		  LocatorManager.cs \
		  LocatorTable.cs \
		  LoggerI.cs \
		  Network.cs \
		  NonRepeatable.cs \
		  ObjectAdapterFactory.cs \
		  ObjectAdapterI.cs \
		  Object.cs \
		  Object_Del.cs \
		  Object_DelD.cs \
		  Object_DelM.cs \
		  ObjectFactoryManager.cs \
		  ObjectImpl.cs \
		  ObjectPrx.cs \
		  ObjectPrxHelper.cs \
		  ObjectPrxHelperBase.cs \
		  OutgoingAsync.cs \
		  OutgoingConnectionFactory.cs \
		  Outgoing.cs \
		  OutputBase.cs \
		  Patcher.cs \
		  PluginFactory.cs \
		  PluginManagerI.cs \
		  PropertiesI.cs \
		  Protocol.cs \
		  ProxyFactory.cs \
		  Reference.cs \
		  ReferenceFactory.cs \
		  RouterInfo.cs \
		  RouterManager.cs \
		  RoutingTable.cs \
		  ServantManager.cs \
		  Set.cs \
		  StringUtil.cs \
		  SupportClass.cs \
		  SysLoggerI.cs \
		  TcpAcceptor.cs \
		  TcpConnector.cs \
		  TcpEndpoint.cs \
		  TcpEndpointFactory.cs \
		  TcpTransceiver.cs \
		  ThreadPool.cs \
		  TraceLevels.cs \
		  TraceUtil.cs \
		  Transceiver.cs \
		  UdpEndpoint.cs \
		  UdpEndpointFactory.cs \
		  UdpTransceiver.cs \
		  UnknownEndpoint.cs \
		  UserExceptionFactory.cs \
		  UserExceptionFactoryManager.cs \
		  Util.cs \
		  ValueWriter.cs \
		  Version.cs \
		  XMLOutput.cs

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/CommunicatorF.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/LocalException.ice \
		  $(SDIR)/LocatorF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LoggerF.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/ObjectAdapterF.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/ObjectFactoryF.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/PluginF.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/ProcessF.ice \
		  $(SDIR)/Process.ice \
		  $(SDIR)/PropertiesF.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/ServantLocatorF.ice \
		  $(SDIR)/ServantLocator.ice \
		  $(SDIR)/StatsF.ice \
		  $(SDIR)/Stats.ice

SDIR		= $(slicedir)/Ice
GDIR		= generated

include $(top_srcdir)/config/Make.rules

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $^

install:: all
	$(INSTALL) $(libdir)/$(LIBNAME) $(install_libdir)

include .depend
