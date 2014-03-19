# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\ice$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\ice$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= Acceptor.obj \
		  Application.obj \
                  Base64.obj \
		  Buffer.obj \
		  BasicStream.obj \
		  BuiltinSequences.obj \
		  CommunicatorI.obj \
		  Communicator.obj \
		  ConnectRequestHandler.obj \
		  ConnectionFactory.obj \
		  ConnectionI.obj \
		  ConnectionMonitor.obj \
		  Connection.obj \
		  Connector.obj \
		  ConnectionRequestHandler.obj \
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
		  EndpointTypes.obj \
		  EventHandler.obj \
		  Exception.obj \
		  FacetMap.obj \
		  FactoryTable.obj \
		  FactoryTableInit.obj \
		  GC.obj \
		  Identity.obj \
		  ImplicitContextI.obj \
		  ImplicitContext.obj \
		  IncomingAsync.obj \
		  Incoming.obj \
		  Initialize.obj \
		  Instance.obj \
		  IPEndpointI.obj \
		  LocalException.obj \
		  LocalObject.obj \
		  LocatorInfo.obj \
		  Locator.obj \
		  LoggerI.obj \
		  Logger.obj \
		  LoggerUtil.obj \
		  Metrics.obj \
		  MetricsAdminI.obj \
		  MetricsObserverI.obj \
		  Network.obj \
		  ObjectAdapterFactory.obj \
		  ObjectAdapterI.obj \
		  ObjectAdapter.obj \
		  ObjectFactoryManager.obj \
		  ObjectFactory.obj \
		  Object.obj \
		  Instrumentation.obj \
		  ObserverHelper.obj \
		  InstrumentationI.obj \
		  OpaqueEndpointI.obj \
		  OutgoingAsync.obj \
		  Outgoing.obj \
		  PluginManagerI.obj \
		  Plugin.obj \
		  Process.obj \
		  PropertiesAdminI.obj \
		  PropertiesI.obj \
		  PropertiesAdmin.obj \
		  Properties.obj \
		  PropertyNames.obj \
		  Protocol.obj \
		  ProtocolInstance.obj \
		  ProtocolPluginFacade.obj \
		  ProxyFactory.obj \
		  Proxy.obj \
		  ReferenceFactory.obj \
		  Reference.obj \
		  RetryQueue.obj \
		  RequestHandler.obj \
		  RouterInfo.obj \
		  Router.obj \
		  Selector.obj \
		  ServantLocator.obj \
		  ServantManager.obj \
		  Service.obj \
		  SliceChecksumDict.obj \
		  SliceChecksums.obj \
		  SlicedData.obj \
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
		  Version.obj

SRCS		= $(OBJS:.obj=.cpp)

HDIR		= $(headerdir)\Ice
SDIR		= $(slicedir)\Ice

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DWIN32_LEAN_AND_MEAN
!if "$(UNIQUE_DLL_NAMES)" == "yes"
CPPFLAGS	= $(CPPFLAGS) -DCOMPSUFFIX=\"$(COMPSUFFIX)\"
!endif
SLICE2CPPFLAGS	= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)
LINKWITH        = $(BASELIBS) $(BZIP2_LIBS) $(ICE_OS_LIBS) ws2_32.lib Iphlpapi.lib

PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
LD_DLLFLAGS	= $(LD_DLLFLAGS) /entry:"ice_DLL_Main"
RES_FILE	= Ice.res


$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) Ice.res
	$(LINK) $(BASE):0x22000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

Service.obj: EventLoggerMsg.h

Ice.res: EventLoggerMsg.rc

# These files are not automatically generated because VC2008 Express doesn't have mc.exe
#EventLoggerMsg.h EventLoggerMsg.rc: EventLoggerMsg.mc
#	mc EventLoggerMsg.mc

clean::
	-del /q BuiltinSequences.cpp $(HDIR)\BuiltinSequences.h
	-del /q CommunicatorF.cpp $(HDIR)\CommunicatorF.h
	-del /q Communicator.cpp $(HDIR)\Communicator.h
	-del /q ConnectionF.cpp $(HDIR)\ConnectionF.h
	-del /q Connection.cpp $(HDIR)\Connection.h
	-del /q Current.cpp $(HDIR)\Current.h
	-del /q Endpoint.cpp $(HDIR)\Endpoint.h
	-del /q EndpointF.cpp $(HDIR)\EndpointF.h
	-del /q EndpointTypes.cpp $(HDIR)\EndpointTypes.h
	-del /q FacetMap.cpp $(HDIR)\FacetMap.h
	-del /q ImplicitContextF.cpp $(HDIR)\ImplicitContextF.h	
	-del /q ImplicitContext.cpp $(HDIR)\ImplicitContext.h	
	-del /q Identity.cpp $(HDIR)\Identity.h
	-del /q LocalException.cpp $(HDIR)\LocalException.h
	-del /q LocatorF.cpp $(HDIR)\LocatorF.h
	-del /q Locator.cpp $(HDIR)\Locator.h
	-del /q LoggerF.cpp $(HDIR)\LoggerF.h
	-del /q Logger.cpp $(HDIR)\Logger.h
	-del /q Metrics.cpp $(HDIR)\Metrics.h
	-del /q ObjectAdapterF.cpp $(HDIR)\ObjectAdapterF.h
	-del /q ObjectAdapter.cpp $(HDIR)\ObjectAdapter.h
	-del /q Instrumentation.cpp $(HDIR)\Instrumentation.h
	-del /q InstrumentationF.cpp $(HDIR)\InstrumentationF.h
	-del /q ObjectFactoryF.cpp $(HDIR)\ObjectFactoryF.h
	-del /q ObjectFactory.cpp $(HDIR)\ObjectFactory.h
	-del /q PluginF.cpp $(HDIR)\PluginF.h
	-del /q Plugin.cpp $(HDIR)\Plugin.h
	-del /q ProcessF.cpp $(HDIR)\ProcessF.h
	-del /q Process.cpp $(HDIR)\Process.h
	-del /q PropertiesF.cpp $(HDIR)\PropertiesF.h
	-del /q PropertiesAdmin.cpp $(HDIR)\PropertiesAdmin.h
	-del /q Properties.cpp $(HDIR)\Properties.h
	-del /q RouterF.cpp $(HDIR)\RouterF.h
	-del /q Router.cpp $(HDIR)\Router.h
	-del /q ServantLocatorF.cpp $(HDIR)\ServantLocatorF.h
	-del /q ServantLocator.cpp $(HDIR)\ServantLocator.h
	-del /q SliceChecksumDict.cpp $(HDIR)\SliceChecksumDict.h
	-del /q StatsF.cpp $(HDIR)\StatsF.h
	-del /q Stats.cpp $(HDIR)\Stats.h
        -del /q Version.cpp $(HDIR)\Version.h
	-del /q Ice.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif

!include .depend.mak
