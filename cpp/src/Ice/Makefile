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

NAME		= $(top_srcdir)/lib/libIce.$(LIBEXT)
VERSIONED_NAME	= $(NAME).$(VERSION)

TARGETS		= $(NAME) $(VERSIONED_NAME)

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
		  Stream.o \
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
		  UserExceptionFactory.o \
		  UserExceptionFactoryManager.o \
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
		  StringUtil.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/LocalException.ice \
		  $(SDIR)/Communicator.ice \
		  $(SDIR)/CommunicatorF.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/LoggerF.ice \
		  $(SDIR)/Stream.ice \
		  $(SDIR)/StreamF.ice \
		  $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/ObjectAdapterF.ice \
		  $(SDIR)/ServantLocator.ice \
		  $(SDIR)/ServantLocatorF.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/PropertiesF.ice \
		  $(SDIR)/ObjectFactory.ice \
		  $(SDIR)/ObjectFactoryF.ice \
		  $(SDIR)/UserExceptionFactory.ice \
		  $(SDIR)/UserExceptionFactoryF.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/Facet.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Plugin.ice \
		  $(SDIR)/PluginF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LocatorF.ice

HDIR		= $(includedir)/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)
SLICE2CPPFLAGS	:= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)

$(VERSIONED_NAME): $(OBJS)
	rm -f $@
	$(CXX) $(SHLIB_LDFLAGS) $(LDFLAGS) -o $@ $(OBJS)

install:: all
	$(INSTALL_LIBRARY) $(VERSIONED_NAME) $(install_libdir)
	rm -f $(install_libdir)/$(notdir $(NAME))
	ln -s $(notdir $(VERSIONED_NAME)) $(install_libdir)/$(notdir $(NAME))

include .depend
