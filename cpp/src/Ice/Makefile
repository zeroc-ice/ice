# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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

OBJS		= Initialize.o \
		  Application.o \
		  Exception.o \
		  LocalException.o \
		  Properties.o \
		  PropertiesI.o \
		  Logger.o \
		  LoggerUtil.o \
		  LoggerI.o \
		  Stats.o \
		  SysLoggerI.o \
		  BuiltinSequences.o \
		  BasicStream.o \
		  DefaultsAndOverrides.o \
		  TraceLevels.o \
		  TraceUtil.o \
		  Instance.o \
		  Communicator.o \
		  CommunicatorI.o \
		  ObjectFactory.o \
		  ObjectFactoryManager.o \
		  ObjectAdapter.o \
		  ObjectAdapterI.o \
		  ObjectAdapterFactory.o \
		  ServantManager.o \
		  ServantLocator.o \
		  Endpoint.o \
		  EndpointFactory.o \
		  EndpointFactoryManager.o \
		  TcpEndpoint.o \
		  UdpEndpoint.o \
		  UnknownEndpoint.o \
		  Reference.o \
		  ReferenceFactory.o \
		  Identity.o \
		  IdentityUtil.o \
		  Current.o \
		  Router.o \
		  RouterInfo.o \
		  RoutingTable.o \
		  ConnectionI.o \
		  ConnectionFactory.o \
		  ConnectionMonitor.o \
		  LocalObject.o \
		  Object.o \
		  ProxyFactory.o \
		  Proxy.o \
		  Outgoing.o \
		  OutgoingAsync.o \
		  Incoming.o \
		  IncomingAsync.o \
		  Direct.o \
		  Network.o \
	          ThreadPool.o \
		  EventHandler.o \
		  Connector.o \
		  Acceptor.o \
		  Transceiver.o \
		  TcpConnector.o \
		  TcpAcceptor.o \
		  TcpTransceiver.o \
		  UdpTransceiver.o \
		  TransportInfo.o \
		  TransportInfoI.o \
		  DynamicLibrary.o \
		  Plugin.o \
		  PluginManagerI.o \
		  ProtocolPluginFacade.o \
		  Locator.o \
		  LocatorInfo.o \
		  FactoryTableDef.o \
		  FactoryTable.o \
		  Service.o \
		  Process.o \
		  FacetMap.o \
		  SliceChecksumDict.o \
		  SliceChecksums.o \
		  Stream.o \
		  StreamI.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/LocalException.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/CommunicatorF.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/LoggerF.ice \
		  $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/ObjectAdapterF.ice \
		  $(SDIR)/ServantLocator.ice \
		  $(SDIR)/ServantLocatorF.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/PropertiesF.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/ObjectFactoryF.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/PluginF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LocatorF.ice \
                  $(SDIR)/StatsF.ice \
                  $(SDIR)/Stats.ice \
                  $(SDIR)/Process.ice \
                  $(SDIR)/ProcessF.ice \
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/SliceChecksumDict.ice \
		  $(SDIR)/TransportInfo.ice \
		  $(SDIR)/TransportInfoF.ice

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
