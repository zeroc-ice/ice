# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
                  DispatchInterceptor.obj \
		  DLLMain.obj \
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
		  Identity.obj \
		  IdentityUtil.obj \
		  ImplicitContextI.obj \
		  ImplicitContext.obj \
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
		  UdpConnector.obj \
		  UdpEndpointI.obj \
		  UdpTransceiver.obj \
		  UnknownEndpointI.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(includedir)\Ice
SDIR		= $(slicedir)\Ice

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DFD_SETSIZE=1024 -DWIN32_LEAN_AND_MEAN
SLICE2CPPFLAGS	= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)
LINKWITH        = $(BASELIBS) $(BZIP2_LIBS) $(ICE_OS_LIBS) ws2_32.lib

!if "$(CPP_COMPILER)" == "BCC2006"
RES_FILE	= ,, EventLoggerMsg.res
!else
!if "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif
LD_DLLFLAGS	= $(LD_DLLFLAGS) /entry:"ice_DLL_Main"
RES_FILE	= EventLoggerMsg.res
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

EventLoggerI.obj: EventLoggerMsg.h

EventLoggerMsg.h EventLoggerMsg.res: EventLoggerMsg.mc
	mc EventLoggerMsg.mc
	$(RC) -r -fo EventLoggerMsg.res EventLoggerMsg.rc

clean::
	del /q BuiltinSequences.cpp $(HDIR)\BuiltinSequences.h
	del /q CommunicatorF.cpp $(HDIR)\CommunicatorF.h
	del /q Communicator.cpp $(HDIR)\Communicator.h
	del /q ConnectionF.cpp $(HDIR)\ConnectionF.h
	del /q Connection.cpp $(HDIR)\Connection.h
	del /q Current.cpp $(HDIR)\Current.h
	del /q Endpoint.cpp $(HDIR)\Endpoint.h
	del /q FacetMap.cpp $(HDIR)\FacetMap.h
	del /q ImplicitContextF.cpp $(HDIR)\ImplicitContextF.h	
	del /q ImplicitContext.cpp $(HDIR)\ImplicitContext.h	
	del /q Identity.cpp $(HDIR)\Identity.h
	del /q LocalException.cpp $(HDIR)\LocalException.h
	del /q LocatorF.cpp $(HDIR)\LocatorF.h
	del /q Locator.cpp $(HDIR)\Locator.h
	del /q LoggerF.cpp $(HDIR)\LoggerF.h
	del /q Logger.cpp $(HDIR)\Logger.h
	del /q ObjectAdapterF.cpp $(HDIR)\ObjectAdapterF.h
	del /q ObjectAdapter.cpp $(HDIR)\ObjectAdapter.h
	del /q ObjectFactoryF.cpp $(HDIR)\ObjectFactoryF.h
	del /q ObjectFactory.cpp $(HDIR)\ObjectFactory.h
	del /q PluginF.cpp $(HDIR)\PluginF.h
	del /q Plugin.cpp $(HDIR)\Plugin.h
	del /q ProcessF.cpp $(HDIR)\ProcessF.h
	del /q Process.cpp $(HDIR)\Process.h
	del /q PropertiesF.cpp $(HDIR)\PropertiesF.h
	del /q Properties.cpp $(HDIR)\Properties.h
	del /q RouterF.cpp $(HDIR)\RouterF.h
	del /q Router.cpp $(HDIR)\Router.h
	del /q ServantLocatorF.cpp $(HDIR)\ServantLocatorF.h
	del /q ServantLocator.cpp $(HDIR)\ServantLocator.h
	del /q SliceChecksumDict.cpp $(HDIR)\SliceChecksumDict.h
	del /q StatsF.cpp $(HDIR)\StatsF.h
	del /q Stats.cpp $(HDIR)\Stats.h
	del /q $(DLLNAME:.dll=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)

!endif

!endif

!include .depend
