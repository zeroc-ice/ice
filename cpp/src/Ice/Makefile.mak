# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\ice$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\ice$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= Acceptor.obj \
		  Application.obj \
		  Buffer.obj \
		  BasicStream.obj \
		  BuiltinSequences.obj \
		  CommunicatorI.obj \
		  Communicator.obj \
		  ConnectionFactory.obj \
		  ConnectionI.obj \
		  ConnectionMonitor.obj \
		  Connection.obj \
		  Connector.obj \
		  Current.obj \
		  DefaultsAndOverrides.obj \
		  Direct.obj \
		  DynamicLibrary.obj \
		  EndpointFactoryManager.obj \
		  EndpointFactory.obj \
		  Endpoint.obj \
		  EndpointI.obj \
		  EventHandler.obj \
		  EventLoggerI.obj \
		  Exception.obj \
		  FacetMap.obj \
		  FactoryTableDef.obj \
		  FactoryTable.obj \
		  GC.obj \
		  GCRecMutex.obj \
		  GCShared.obj \
		  Identity.obj \
		  IdentityUtil.obj \
		  IncomingAsync.obj \
		  Incoming.obj \
		  Initialize.obj \
		  Instance.obj \
		  LocalException.obj \
		  LocalObject.obj \
		  LocatorInfo.obj \
		  Locator.obj \
		  LoggerI.obj \
		  Logger.obj \
		  LoggerUtil.obj \
		  Network.obj \
		  ObjectAdapterFactory.obj \
		  ObjectAdapterI.obj \
		  ObjectAdapter.obj \
		  ObjectFactoryManager.obj \
		  ObjectFactory.obj \
		  Object.obj \
		  OutgoingAsync.obj \
		  Outgoing.obj \
		  PluginManagerI.obj \
		  Plugin.obj \
		  Process.obj \
		  PropertiesI.obj \
		  Properties.obj \
		  PropertyNames.obj \
		  Protocol.obj \
		  ProtocolPluginFacade.obj \
		  ProxyFactory.obj \
		  Proxy.obj \
		  ReferenceFactory.obj \
		  Reference.obj \
		  RouterInfo.obj \
		  Router.obj \
		  ServantLocator.obj \
		  ServantManager.obj \
		  Service.obj \
		  SliceChecksumDict.obj \
		  SliceChecksums.obj \
		  Stats.obj \
		  StreamI.obj \
		  Stream.obj \
                  StringConverter.obj \
		  TcpAcceptor.obj \
		  TcpConnector.obj \
		  TcpEndpointI.obj \
		  TcpTransceiver.obj \
	          ThreadPool.obj \
		  TraceLevels.obj \
		  TraceUtil.obj \
		  Transceiver.obj \
		  UdpEndpointI.obj \
		  UdpTransceiver.obj \
		  UnknownEndpointI.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(includedir)\Ice
SDIR		= $(slicedir)\Ice

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DFD_SETSIZE=1024
SLICE2CPPFLAGS	= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)
LINKWITH        = $(BASELIBS) $(BZIP2_LIBS) $(ICE_OS_LIBS) ws2_32.lib

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)

EventLoggerI.obj: EventLoggerMsg.h

EventLoggerMsg.h EventLoggerMsg.res: EventLoggerMsg.mc
	mc EventLoggerMsg.mc
	rc -r -fo EventLoggerMsg.res EventLoggerMsg.rc

BuiltinSequences.cpp $(HDIR)\BuiltinSequences.h: $(SDIR)\BuiltinSequences.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\BuiltinSequences.ice
	move BuiltinSequences.h $(HDIR)

$(HDIR)\CommunicatorF.h: $(SDIR)\CommunicatorF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\CommunicatorF.ice
	move CommunicatorF.h $(HDIR)
	del /q CommunicatorF.cpp

Communicator.cpp $(HDIR)\Communicator.h: $(SDIR)\Communicator.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Communicator.ice
	move Communicator.h $(HDIR)

$(HDIR)\ConnectionF.h: $(SDIR)\ConnectionF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\ConnectionF.ice
	move ConnectionF.h $(HDIR)
	del /q ConnectionF.cpp

Connection.cpp $(HDIR)\Connection.h: $(SDIR)\Connection.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Connection.ice
	move Connection.h $(HDIR)

Current.cpp $(HDIR)\Current.h: $(SDIR)\Current.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Current.ice
	move Current.h $(HDIR)

Endpoint.cpp $(HDIR)\Endpoint.h: $(SDIR)\Endpoint.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Endpoint.ice
	move Endpoint.h $(HDIR)

FacetMap.cpp $(HDIR)\FacetMap.h: $(SDIR)\FacetMap.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\FacetMap.ice
	move FacetMap.h $(HDIR)

Identity.cpp $(HDIR)\Identity.h: $(SDIR)\Identity.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Identity.ice
	move Identity.h $(HDIR)

LocalException.cpp $(HDIR)\LocalException.h: $(SDIR)\LocalException.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\LocalException.ice
	move LocalException.h $(HDIR)

$(HDIR)\LocatorF.h: $(SDIR)\LocatorF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\LocatorF.ice
	move LocatorF.h $(HDIR)
	del /q LocatorF.cpp

Locator.cpp $(HDIR)\Locator.h: $(SDIR)\Locator.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Locator.ice
	move Locator.h $(HDIR)

$(HDIR)\LoggerF.h: $(SDIR)\LoggerF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\LoggerF.ice
	move LoggerF.h $(HDIR)
	del /q LoggerF.cpp

Logger.cpp $(HDIR)\Logger.h: $(SDIR)\Logger.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Logger.ice
	move Logger.h $(HDIR)

$(HDIR)\ObjectAdapterF.h: $(SDIR)\ObjectAdapterF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\ObjectAdapterF.ice
	move ObjectAdapterF.h $(HDIR)
	del /q ObjectAdapterF.cpp

ObjectAdapter.cpp $(HDIR)\ObjectAdapter.h: $(SDIR)\ObjectAdapter.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\ObjectAdapter.ice
	move ObjectAdapter.h $(HDIR)

$(HDIR)\ObjectFactoryF.h: $(SDIR)\ObjectFactoryF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\ObjectFactoryF.ice
	move ObjectFactoryF.h $(HDIR)
	del /q ObjectFactoryF.cpp

ObjectFactory.cpp $(HDIR)\ObjectFactory.h: $(SDIR)\ObjectFactory.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\ObjectFactory.ice
	move ObjectFactory.h $(HDIR)

$(HDIR)\PluginF.h: $(SDIR)\PluginF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\PluginF.ice
	move PluginF.h $(HDIR)
	del /q PluginF.cpp

Plugin.cpp $(HDIR)\Plugin.h: $(SDIR)\Plugin.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Plugin.ice
	move Plugin.h $(HDIR)

$(HDIR)\ProcessF.h: $(SDIR)\ProcessF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\ProcessF.ice
	move ProcessF.h $(HDIR)
	del /q ProcessF.cpp

Process.cpp $(HDIR)\Process.h: $(SDIR)\Process.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Process.ice
	move Process.h $(HDIR)

$(HDIR)\PropertiesF.h: $(SDIR)\PropertiesF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\PropertiesF.ice
	move PropertiesF.h $(HDIR)
	del /q PropertiesF.cpp

Properties.cpp $(HDIR)\Properties.h: $(SDIR)\Properties.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Properties.ice
	move Properties.h $(HDIR)

$(HDIR)\RouterF.h: $(SDIR)\RouterF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\RouterF.ice
	move RouterF.h $(HDIR)
	del /q RouterF.cpp

Router.cpp $(HDIR)\Router.h: $(SDIR)\Router.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Router.ice
	move Router.h $(HDIR)

$(HDIR)\ServantLocatorF.h: $(SDIR)\ServantLocatorF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\ServantLocatorF.ice
	move ServantLocatorF.h $(HDIR)
	del /q ServantLocatorF.cpp

ServantLocator.cpp $(HDIR)\ServantLocator.h: $(SDIR)\ServantLocator.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\ServantLocator.ice
	move ServantLocator.h $(HDIR)

SliceChecksumDict.cpp $(HDIR)\SliceChecksumDict.h: $(SDIR)\SliceChecksumDict.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\SliceChecksumDict.ice
	move SliceChecksumDict.h $(HDIR)

$(HDIR)\StatsF.h: $(SDIR)\StatsF.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\StatsF.ice
	move StatsF.h $(HDIR)
	del /q StatsF.cpp

Stats.cpp $(HDIR)\Stats.h: $(SDIR)\Stats.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)\Stats.ice
	move Stats.h $(HDIR)

clean::
	del /q BuiltinSequences.cpp $(HDIR)\BuiltinSequences.h
	del /q $(HDIR)\CommunicatorF.h
	del /q Communicator.cpp $(HDIR)\Communicator.h
	del /q $(HDIR)\ConnectionF.h
	del /q Connection.cpp $(HDIR)\Connection.h
	del /q Current.cpp $(HDIR)\Current.h
	del /q Endpoint.cpp $(HDIR)\Endpoint.h
	del /q FacetMap.cpp $(HDIR)\FacetMap.h
	del /q Identity.cpp $(HDIR)\Identity.h
	del /q LocalException.cpp $(HDIR)\LocalException.h
	del /q $(HDIR)\LocatorF.h
	del /q Locator.cpp $(HDIR)\Locator.h
	del /q $(HDIR)\LoggerF.h
	del /q Logger.cpp $(HDIR)\Logger.h
	del /q $(HDIR)\ObjectAdapterF.h
	del /q ObjectAdapter.cpp $(HDIR)\ObjectAdapter.h
	del /q $(HDIR)\ObjectFactoryF.h
	del /q ObjectFactory.cpp $(HDIR)\ObjectFactory.h
	del /q $(HDIR)\PluginF.h
	del /q Plugin.cpp $(HDIR)\Plugin.h
	del /q $(HDIR)\ProcessF.h
	del /q Process.cpp $(HDIR)\Process.h
	del /q $(HDIR)\PropertiesF.h
	del /q Properties.cpp $(HDIR)\Properties.h
	del /q $(HDIR)\RouterF.h
	del /q Router.cpp $(HDIR)\Router.h
	del /q $(HDIR)\ServantLocatorF.h
	del /q ServantLocator.cpp $(HDIR)\ServantLocator.h
	del /q SliceChecksumDict.cpp $(HDIR)\SliceChecksumDict.h
	del /q $(HDIR)\StatsF.h
	del /q Stats.cpp $(HDIR)\Stats.h
	del /q $(DLLNAME:.dll=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!include .depend
