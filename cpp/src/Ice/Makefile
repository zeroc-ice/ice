# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,Ice,$(VERSION))
SONAME          = $(call mksoname,Ice,$(SOVERSION))  
LIBNAME		= $(call mklibname,Ice)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= Acceptor.o \
		  Application.o \
		  Buffer.o \
		  BasicStream.o \
		  BuiltinSequences.o \
		  CommunicatorF.o \
		  CommunicatorI.o \
		  Communicator.o \
		  ConnectionFactory.o \
		  ConnectionF.o \
		  ConnectionI.o \
		  ConnectionMonitor.o \
		  Connection.o \
		  Connector.o \
		  Current.o \
		  DefaultsAndOverrides.o \
		  Direct.o \
		  DynamicLibrary.o \
		  EndpointFactoryManager.o \
		  EndpointFactory.o \
		  Endpoint.o \
		  EventHandler.o \
		  Exception.o \
		  FacetMap.o \
		  FactoryTableDef.o \
		  FactoryTable.o \
		  Identity.o \
		  IdentityUtil.o \
		  IncomingAsync.o \
		  Incoming.o \
		  Initialize.o \
		  Instance.o \
		  LocalException.o \
		  LocalObject.o \
		  LocatorF.o \
		  LocatorInfo.o \
		  Locator.o \
		  LoggerF.o \
		  LoggerI.o \
		  Logger.o \
		  LoggerUtil.o \
		  Network.o \
		  ObjectAdapterFactory.o \
		  ObjectAdapterF.o \
		  ObjectAdapterI.o \
		  ObjectAdapter.o \
		  ObjectFactoryF.o \
		  ObjectFactoryManager.o \
		  ObjectFactory.o \
		  Object.o \
		  OutgoingAsync.o \
		  Outgoing.o \
		  PluginF.o \
		  PluginManagerI.o \
		  Plugin.o \
		  ProcessF.o \
		  Process.o \
		  PropertiesF.o \
		  PropertiesI.o \
		  Properties.o \
		  PropertyNames.o \
		  ProtocolPluginFacade.o \
		  ProxyFactory.o \
		  Proxy.o \
		  ReferenceFactory.o \
		  Reference.o \
		  RouterF.o \
		  RouterInfo.o \
		  Router.o \
		  RoutingTable.o \
		  ServantLocatorF.o \
		  ServantLocator.o \
		  ServantManager.o \
		  Service.o \
		  SliceChecksumDict.o \
		  SliceChecksums.o \
		  StatsF.o \
		  Stats.o \
		  StreamI.o \
		  Stream.o \
		  SysLoggerI.o \
		  TcpAcceptor.o \
		  TcpConnector.o \
		  TcpEndpoint.o \
		  TcpTransceiver.o \
	          ThreadPool.o \
		  TraceLevels.o \
		  TraceUtil.o \
		  Transceiver.o \
		  UdpEndpoint.o \
		  UdpTransceiver.o \
		  UnknownEndpoint.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/CommunicatorF.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/ConnectionF.ice \
		  $(SDIR)/Connection.ice \
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
		  $(SDIR)/SliceChecksumDict.ice \
		  $(SDIR)/StatsF.ice \
		  $(SDIR)/Stats.ice

HDIR		= $(includedir)/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DICE_PROTOCOL_API_EXPORTS $(BZIP2_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)

LINKWITH        := -lIceUtil $(BZIP2_LIBS) $(ICE_OS_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
