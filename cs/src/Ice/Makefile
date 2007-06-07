# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PKG		= icecs
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)/$(LIBNAME)

SRCS		= Acceptor.cs \
		  Application.cs \
		  Arrays.cs \
		  AssemblyInfo.cs \
		  AssemblyUtil.cs \
                  Base64.cs \
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
		  Property.cs \
		  PropertyNames.cs \
		  Protocol.cs \
		  ProtocolPluginFacade.cs \
		  Proxy.cs \
		  ProxyFactory.cs \
		  ProxyIdentityKey.cs \
		  ReadObjectCallback.cs \
		  Reference.cs \
		  ReferenceFactory.cs \
                  ReplyStatus.cs \
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
		  UdpConnector.cs \
		  UdpTransceiver.cs \
		  UnknownEndpointI.cs \
		  UserExceptionFactory.cs \
		  Util.cs \
		  ValueWriter.cs \
		  XMLOutput.cs

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/Connection.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/Endpoint.ice \
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/ImplicitContext.ice \
		  $(SDIR)/LocalException.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/Process.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/ServantLocator.ice \
		  $(SDIR)/SliceChecksumDict.ice \
		  $(SDIR)/Stats.ice

SDIR		= $(slicedir)/Ice
GDIR		= generated

include $(top_srcdir)/config/Make.rules.cs

MCSFLAGS	:= $(MCSFLAGS) -target:library -out:$(TARGETS) -unsafe -warnaserror-
MCSFLAGS	:= $(MCSFLAGS) -keyfile:$(top_srcdir)/config/IcecsKey.snk

SLICE2CSFLAGS	:= $(SLICE2CSFLAGS) --ice -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) $(subst /,$(DSEP),$^)

ifeq ($(NOGAC),)

install:: all
	$(GACUTIL) -i $(bindir)/$(LIBNAME)

else

install:: all
	$(call installlibrary,$(bindir)/icecs.dll,$(install_bindir))

endif

include .depend
