#**********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,Ice,$(VERSION))
SONAME          = $(call mksoname,Ice,$(SOVERSION))  
LIBNAME		= $(call mklibname,Ice)

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME)

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
		  Facet.o \
		  Current.o \
		  Router.o \
		  RouterInfo.o \
		  RoutingTable.o \
		  Connection.o \
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
		  DynamicLibrary.o \
		  Plugin.o \
		  PluginManagerI.o \
		  ProtocolPluginFacade.o \
		  Locator.o \
		  LocatorInfo.o \
		  StringUtil.o \
		  FactoryTableDef.o \
		  FactoryTable.o \
		  Service.o

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
		  $(SDIR)/Facet.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/PluginF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LocatorF.ice \
                  $(SDIR)/StatsF.ice \
                  $(SDIR)/Stats.ice

HDIR		= $(includedir)/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DICE_PROTOCOL_API_EXPORTS $(BZIP2_FLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)

LINKWITH        := $(BZIP2_LIBS) $(ICE_OS_LIBS)

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
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)

include .depend
