# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\ice$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\ice$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

SLICE_OBJS	= .\BuiltinSequences.obj \
		  .\Communicator.obj \
		  .\CommunicatorF.obj \
		  .\Connection.obj \
		  .\ConnectionF.obj \
		  .\Current.obj \
		  .\Endpoint.obj \
		  .\EndpointF.obj \
		  .\EndpointTypes.obj \
		  .\FacetMap.obj \
		  .\Identity.obj \
		  .\ImplicitContext.obj \
		  .\ImplicitContextF.obj \
		  .\Instrumentation.obj \
		  .\InstrumentationF.obj \
		  .\LocalException.obj \
		  .\Locator.obj \
		  .\LocatorF.obj \
		  .\Logger.obj \
		  .\LoggerF.obj \
		  .\Metrics.obj \
		  .\ObjectAdapter.obj \
		  .\ObjectAdapterF.obj \
		  .\ObjectFactory.obj \
		  .\ObjectFactoryF.obj \
		  .\Plugin.obj \
		  .\PluginF.obj \
		  .\Process.obj \
		  .\ProcessF.obj \
		  .\Properties.obj \
		  .\PropertiesAdmin.obj \
		  .\PropertiesF.obj \
		  .\RemoteLogger.obj \
		  .\Router.obj \
		  .\RouterF.obj \
		  .\ServantLocator.obj \
		  .\ServantLocatorF.obj \
		  .\SliceChecksumDict.obj \
		  .\Version.obj

WINDOWS_OBJS   =  .\DLLMain.obj

OBJS	       =  .\Acceptor.obj \
		  .\ACM.obj \
		  .\Application.obj \
		  .\AsyncResult.obj \
		  .\Base64.obj \
		  .\BasicStream.obj \
		  .\BatchRequestQueue.obj \
		  .\Buffer.obj \
		  .\CollocatedRequestHandler.obj \
		  .\CommunicatorI.obj \
		  .\ConnectionFactory.obj \
		  .\ConnectionI.obj \
		  .\ConnectionRequestHandler.obj \
		  .\Connector.obj \
		  .\ConnectRequestHandler.obj \
		  .\DefaultsAndOverrides.obj \
		  .\DeprecatedStringConverter.obj \
		  .\DispatchInterceptor.obj \
		  .\DynamicLibrary.obj \
		  .\EndpointFactory.obj \
		  .\EndpointFactoryManager.obj \
		  .\EndpointI.obj \
		  .\EventHandler.obj \
		  .\Exception.obj \
		  .\FactoryTable.obj \
		  .\FactoryTableInit.obj \
		  .\GCObject.obj \
		  .\HttpParser.obj \
		  .\ImplicitContextI.obj \
		  .\Incoming.obj \
		  .\IncomingAsync.obj \
		  .\Initialize.obj \
		  .\Instance.obj \
		  .\InstrumentationI.obj \
		  .\IPEndpointI.obj \
		  .\LocalObject.obj \
		  .\LocatorInfo.obj \
		  .\LoggerAdminI.obj \
		  .\LoggerI.obj \
		  .\LoggerUtil.obj \
		  .\MetricsAdminI.obj \
		  .\MetricsObserverI.obj \
		  .\Network.obj \
		  .\NetworkProxy.obj \
		  .\Object.obj \
		  .\ObjectAdapterFactory.obj \
		  .\ObjectAdapterI.obj \
		  .\ObjectFactoryManager.obj \
		  .\ObserverHelper.obj \
		  .\OpaqueEndpointI.obj \
		  .\Outgoing.obj \
		  .\OutgoingAsync.obj \
		  .\PluginManagerI.obj \
		  .\PropertiesAdminI.obj \
		  .\PropertiesI.obj \
		  .\PropertyNames.obj \
		  .\Protocol.obj \
		  .\ProtocolInstance.obj \
		  .\ProtocolPluginFacade.obj \
		  .\Proxy.obj \
		  .\ProxyFactory.obj \
		  .\Reference.obj \
		  .\ReferenceFactory.obj \
		  .\RegisterPluginsInit.obj \
		  .\RequestHandler.obj \
		  .\RequestHandlerFactory.obj \
		  .\ResponseHandler.obj \
		  .\RetryQueue.obj \
		  .\RouterInfo.obj \
		  .\Selector.obj \
		  .\ServantManager.obj \
		  .\Service.obj \
		  .\SliceChecksums.obj \
		  .\SlicedData.obj \
		  .\Stream.obj \
		  .\StreamI.obj \
		  .\StreamSocket.obj \
		  .\StringConverterPlugin.obj \
		  .\TcpAcceptor.obj \
		  .\TcpConnector.obj \
		  .\TcpEndpointI.obj \
		  .\TcpTransceiver.obj \
		  .\ThreadPool.obj \
		  .\TraceLevels.obj \
		  .\TraceUtil.obj \
		  .\Transceiver.obj \
		  .\UdpConnector.obj \
		  .\UdpEndpointI.obj \
		  .\UdpTransceiver.obj \
		  .\WSAcceptor.obj \
		  .\WSConnector.obj \
		  .\WSEndpoint.obj \
		  .\WSTransceiver.obj \
		  $(WINDOWS_OBJS) \
		  $(SLICE_OBJS)

HDIR		= $(headerdir)\Ice
SDIR		= $(slicedir)\Ice

RC_SRCS		= EventLoggerMsg.rc

!include $(top_srcdir)\config\Make.rules.mak

$(OBJS)		: $(BZIP2_NUPKG)

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DWIN32_LEAN_AND_MEAN -bigobj $(BZIP2_CPPFLAGS)
!if "$(UNIQUE_DLL_NAMES)" == "yes"
CPPFLAGS	= $(CPPFLAGS) -DCOMPSUFFIX=\"$(COMPSUFFIX)\"
!endif
SLICE2CPPFLAGS	= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPFLAGS)
LINKWITH        =  $(BASELIBS) $(BZIP2_LIBS) $(ICE_OS_LIBS)

PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
LD_DLLFLAGS	= $(LD_DLLFLAGS) /entry:"ice_DLL_Main"
RES_FILE	= Ice.res


$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) Ice.res
	$(LINK) $(BASE):0x22000000 $(LD_DLLFLAGS) $(BZIP2_LDFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
		$(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

Ice.res: EventLoggerMsg.rc

EventLoggerMsg.h EventLoggerMsg.rc: EventLoggerMsg.mc
        mc EventLoggerMsg.mc

clean::
	-del /q BuiltinSequences.cpp $(HDIR)\BuiltinSequences.h
	-del /q Communicator.cpp $(HDIR)\Communicator.h
	-del /q CommunicatorF.cpp $(HDIR)\CommunicatorF.h
	-del /q Connection.cpp $(HDIR)\Connection.h
	-del /q ConnectionF.cpp $(HDIR)\ConnectionF.h
	-del /q Current.cpp $(HDIR)\Current.h
	-del /q Endpoint.cpp $(HDIR)\Endpoint.h
	-del /q EndpointF.cpp $(HDIR)\EndpointF.h
	-del /q EndpointTypes.cpp $(HDIR)\EndpointTypes.h
	-del /q FacetMap.cpp $(HDIR)\FacetMap.h
	-del /q Identity.cpp $(HDIR)\Identity.h
	-del /q ImplicitContext.cpp $(HDIR)\ImplicitContext.h
	-del /q ImplicitContextF.cpp $(HDIR)\ImplicitContextF.h
	-del /q Instrumentation.cpp $(HDIR)\Instrumentation.h
	-del /q InstrumentationF.cpp $(HDIR)\InstrumentationF.h
	-del /q LocalException.cpp $(HDIR)\LocalException.h
	-del /q Locator.cpp $(HDIR)\Locator.h
	-del /q LocatorF.cpp $(HDIR)\LocatorF.h
	-del /q Logger.cpp $(HDIR)\Logger.h
	-del /q LoggerF.cpp $(HDIR)\LoggerF.h
	-del /q Metrics.cpp $(HDIR)\Metrics.h
	-del /q ObjectAdapter.cpp $(HDIR)\ObjectAdapter.h
	-del /q ObjectAdapterF.cpp $(HDIR)\ObjectAdapterF.h
	-del /q ObjectFactory.cpp $(HDIR)\ObjectFactory.h
	-del /q ObjectFactoryF.cpp $(HDIR)\ObjectFactoryF.h
	-del /q Plugin.cpp $(HDIR)\Plugin.h
	-del /q PluginF.cpp $(HDIR)\PluginF.h
	-del /q Process.cpp $(HDIR)\Process.h
	-del /q ProcessF.cpp $(HDIR)\ProcessF.h
	-del /q Properties.cpp $(HDIR)\Properties.h
	-del /q PropertiesAdmin.cpp $(HDIR)\PropertiesAdmin.h
	-del /q PropertiesF.cpp $(HDIR)\PropertiesF.h
	-del /q RemoteLogger.cpp $(HDIR)\RemoteLogger.h
	-del /q Router.cpp $(HDIR)\Router.h
	-del /q RouterF.cpp $(HDIR)\RouterF.h
	-del /q ServantLocator.cpp $(HDIR)\ServantLocator.h
	-del /q ServantLocatorF.cpp $(HDIR)\ServantLocatorF.h
	-del /q SliceChecksumDict.cpp $(HDIR)\SliceChecksumDict.h
	-del /q Version.cpp $(HDIR)\Version.h
	-del /q Ice.res
	-del /q EventLoggerMsg.h EventLoggerMsg.rc MSG00001.bin

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif
