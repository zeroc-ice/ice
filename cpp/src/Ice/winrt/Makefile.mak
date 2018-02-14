# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..
LIBNAME		= $(SDK_LIBRARY_PATH)\ice.lib
TARGETS		= $(LIBNAME)
SOURCE_DIR	= ..

OBJS		= $(ARCH)\$(CONFIG)\Acceptor.obj \
		  $(ARCH)\$(CONFIG)\ACM.obj \
		  $(ARCH)\$(CONFIG)\AsyncResult.obj \
		  $(ARCH)\$(CONFIG)\Base64.obj \
		  $(ARCH)\$(CONFIG)\Buffer.obj \
		  $(ARCH)\$(CONFIG)\BasicStream.obj \
		  $(ARCH)\$(CONFIG)\BatchRequestQueue.obj \
		  $(ARCH)\$(CONFIG)\BuiltinSequences.obj \
		  $(ARCH)\$(CONFIG)\CommunicatorI.obj \
		  $(ARCH)\$(CONFIG)\Communicator.obj \
		  $(ARCH)\$(CONFIG)\CollocatedRequestHandler.obj \
		  $(ARCH)\$(CONFIG)\ConnectRequestHandler.obj \
		  $(ARCH)\$(CONFIG)\ConnectionFactory.obj \
		  $(ARCH)\$(CONFIG)\ConnectionI.obj \
		  $(ARCH)\$(CONFIG)\Connection.obj \
		  $(ARCH)\$(CONFIG)\Connector.obj \
		  $(ARCH)\$(CONFIG)\ConnectionRequestHandler.obj \
		  $(ARCH)\$(CONFIG)\Current.obj \
		  $(ARCH)\$(CONFIG)\DefaultsAndOverrides.obj \
		  $(ARCH)\$(CONFIG)\DeprecatedStringConverter.obj \
		  $(ARCH)\$(CONFIG)\DispatchInterceptor.obj \
		  $(ARCH)\$(CONFIG)\DynamicLibrary.obj \
		  $(ARCH)\$(CONFIG)\EndpointFactoryManager.obj \
		  $(ARCH)\$(CONFIG)\EndpointFactory.obj \
		  $(ARCH)\$(CONFIG)\Endpoint.obj \
		  $(ARCH)\$(CONFIG)\EndpointI.obj \
		  $(ARCH)\$(CONFIG)\EndpointTypes.obj \
		  $(ARCH)\$(CONFIG)\EventHandler.obj \
		  $(ARCH)\$(CONFIG)\Exception.obj \
		  $(ARCH)\$(CONFIG)\FacetMap.obj \
		  $(ARCH)\$(CONFIG)\FactoryTable.obj \
		  $(ARCH)\$(CONFIG)\FactoryTableInit.obj \
		  $(ARCH)\$(CONFIG)\GCObject.obj \
		  $(ARCH)\$(CONFIG)\Identity.obj \
		  $(ARCH)\$(CONFIG)\ImplicitContextI.obj \
		  $(ARCH)\$(CONFIG)\ImplicitContext.obj \
		  $(ARCH)\$(CONFIG)\IncomingAsync.obj \
		  $(ARCH)\$(CONFIG)\Incoming.obj \
		  $(ARCH)\$(CONFIG)\Initialize.obj \
		  $(ARCH)\$(CONFIG)\Instance.obj \
		  $(ARCH)\$(CONFIG)\IPEndpointI.obj \
		  $(ARCH)\$(CONFIG)\LocalException.obj \
		  $(ARCH)\$(CONFIG)\LocalObject.obj \
		  $(ARCH)\$(CONFIG)\LocatorInfo.obj \
		  $(ARCH)\$(CONFIG)\Locator.obj \
		  $(ARCH)\$(CONFIG)\LoggerAdminI.obj \
		  $(ARCH)\$(CONFIG)\LoggerI.obj \
		  $(ARCH)\$(CONFIG)\Logger.obj \
		  $(ARCH)\$(CONFIG)\LoggerUtil.obj \
		  $(ARCH)\$(CONFIG)\Metrics.obj \
		  $(ARCH)\$(CONFIG)\MetricsAdminI.obj \
		  $(ARCH)\$(CONFIG)\MetricsObserverI.obj \
		  $(ARCH)\$(CONFIG)\Network.obj \
		  $(ARCH)\$(CONFIG)\NetworkProxy.obj \
		  $(ARCH)\$(CONFIG)\ObjectAdapterFactory.obj \
		  $(ARCH)\$(CONFIG)\ObjectAdapterI.obj \
		  $(ARCH)\$(CONFIG)\ObjectAdapter.obj \
		  $(ARCH)\$(CONFIG)\ObjectFactoryManager.obj \
		  $(ARCH)\$(CONFIG)\ObjectFactory.obj \
		  $(ARCH)\$(CONFIG)\Object.obj \
		  $(ARCH)\$(CONFIG)\Instrumentation.obj \
		  $(ARCH)\$(CONFIG)\InstrumentationF.obj \
		  $(ARCH)\$(CONFIG)\ObserverHelper.obj \
		  $(ARCH)\$(CONFIG)\InstrumentationI.obj \
		  $(ARCH)\$(CONFIG)\OpaqueEndpointI.obj \
		  $(ARCH)\$(CONFIG)\OutgoingAsync.obj \
		  $(ARCH)\$(CONFIG)\Outgoing.obj \
		  $(ARCH)\$(CONFIG)\PluginManagerI.obj \
		  $(ARCH)\$(CONFIG)\Plugin.obj \
		  $(ARCH)\$(CONFIG)\Process.obj \
		  $(ARCH)\$(CONFIG)\PropertiesI.obj \
		  $(ARCH)\$(CONFIG)\Properties.obj \
		  $(ARCH)\$(CONFIG)\PropertiesAdmin.obj \
		  $(ARCH)\$(CONFIG)\PropertiesAdminI.obj \
		  $(ARCH)\$(CONFIG)\PropertyNames.obj \
		  $(ARCH)\$(CONFIG)\Protocol.obj \
		  $(ARCH)\$(CONFIG)\ProtocolInstance.obj \
		  $(ARCH)\$(CONFIG)\ProtocolPluginFacade.obj \
		  $(ARCH)\$(CONFIG)\ProxyFactory.obj \
		  $(ARCH)\$(CONFIG)\Proxy.obj \
		  $(ARCH)\$(CONFIG)\ReferenceFactory.obj \
		  $(ARCH)\$(CONFIG)\Reference.obj \
		  $(ARCH)\$(CONFIG)\RemoteLogger.obj \
		  $(ARCH)\$(CONFIG)\RetryQueue.obj \
		  $(ARCH)\$(CONFIG)\RequestHandler.obj \
		  $(ARCH)\$(CONFIG)\RequestHandlerFactory.obj \
		  $(ARCH)\$(CONFIG)\ResponseHandler.obj \
		  $(ARCH)\$(CONFIG)\RouterInfo.obj \
		  $(ARCH)\$(CONFIG)\Router.obj \
		  $(ARCH)\$(CONFIG)\Selector.obj \
		  $(ARCH)\$(CONFIG)\ServantLocator.obj \
		  $(ARCH)\$(CONFIG)\ServantManager.obj \
		  $(ARCH)\$(CONFIG)\SliceChecksumDict.obj \
		  $(ARCH)\$(CONFIG)\SliceChecksums.obj \
		  $(ARCH)\$(CONFIG)\SlicedData.obj \
		  $(ARCH)\$(CONFIG)\StreamI.obj \
		  $(ARCH)\$(CONFIG)\Stream.obj \
		  $(ARCH)\$(CONFIG)\StringConverterPlugin.obj \
		  $(ARCH)\$(CONFIG)\ThreadPool.obj \
		  $(ARCH)\$(CONFIG)\TraceLevels.obj \
		  $(ARCH)\$(CONFIG)\TraceUtil.obj \
		  $(ARCH)\$(CONFIG)\Transceiver.obj \
		  $(ARCH)\$(CONFIG)\UdpConnector.obj \
		  $(ARCH)\$(CONFIG)\UdpEndpointI.obj \
		  $(ARCH)\$(CONFIG)\UdpTransceiver.obj \
		  $(ARCH)\$(CONFIG)\Version.obj \
		  $(ARCH)\$(CONFIG)\WSAcceptor.obj \
		  $(ARCH)\$(CONFIG)\WSConnector.obj \
		  $(ARCH)\$(CONFIG)\WSEndpoint.obj \
		  $(ARCH)\$(CONFIG)\WSTransceiver.obj \
		  $(ARCH)\$(CONFIG)\HttpParser.obj \
		  $(ARCH)\$(CONFIG)\IceDiscovery\LookupI.obj \
		  $(ARCH)\$(CONFIG)\IceDiscovery\LocatorI.obj \
		  $(ARCH)\$(CONFIG)\IceDiscovery\PluginI.obj \
		  $(ARCH)\$(CONFIG)\IceLocatorDiscovery\PluginI.obj

LOCAL_OBJS	= $(ARCH)\$(CONFIG)\RegisterPluginsInit.obj \
		  $(ARCH)\$(CONFIG)\StreamAcceptor.obj \
		  $(ARCH)\$(CONFIG)\StreamConnector.obj \
		  $(ARCH)\$(CONFIG)\StreamEndpointI.obj \
		  $(ARCH)\$(CONFIG)\StreamTransceiver.obj \
		  $(ARCH)\$(CONFIG)\EndpointInfo.obj \
		  $(ARCH)\$(CONFIG)\ConnectionInfo.obj \
		  $(ARCH)\$(CONFIG)\IceDiscovery.obj \
		  $(ARCH)\$(CONFIG)\IceLocatorDiscovery.obj

SLICE_SRCS	= Ice\BuiltinSequences.ice \
		  Ice\CommunicatorF.ice \
		  Ice\Communicator.ice \
		  Ice\ConnectionF.ice \
		  Ice\Connection.ice \
		  Ice\Current.ice \
		  Ice\Endpoint.ice \
		  Ice\EndpointF.ice \
		  Ice\EndpointTypes.ice \
		  Ice\FacetMap.ice \
		  Ice\Identity.ice \
		  Ice\ImplicitContextF.ice \
		  Ice\ImplicitContext.ice \
		  Ice\LocalException.ice \
		  Ice\LocatorF.ice \
		  Ice\Locator.ice \
		  Ice\LoggerF.ice \
		  Ice\Logger.ice \
		  Ice\ObjectAdapterF.ice \
		  Ice\ObjectAdapter.ice \
		  Ice\ObjectFactoryF.ice \
		  Ice\ObjectFactory.ice \
		  Ice\PluginF.ice \
		  Ice\Plugin.ice \
		  Ice\ProcessF.ice \
		  Ice\Process.ice \
		  Ice\PropertiesF.ice \
		  Ice\Properties.ice \
		  Ice\PropertiesAdmin.ice \
		  Ice\RemoteLogger.ice \
		  Ice\Router.ice \
		  Ice\RouterF.ice \
		  Ice\ServantLocatorF.ice \
		  Ice\ServantLocator.ice \
		  Ice\SliceChecksumDict.ice \
		  Ice\Version.ice \
		  Ice\Metrics.ice \
		  Ice\Instrumentation.ice \
		  IceSSL\EndpointInfo.ice \
		  IceSSL\ConnectionInfo.ice \
		  IceDiscovery\IceDiscovery.ice \
		  IceLocatorDiscovery\IceLocatorDiscovery.ice

SRCS		= $(OBJS:.obj=.cpp)
SRCS		= $(SRCS:x86\=)
SRCS		= $(SRCS:x64\=)
SRCS		= $(SRCS:arm\=)
SRCS		= $(SRCS:Retail\=..\)
SRCS		= $(SRCS:Debug\=..\)
SRCS		= $(SRCS:..\IceDiscovery\=..\..\IceDiscovery\)
SRCS		= $(SRCS:..\IceLocatorDiscovery\=..\..\IceLocatorDiscovery\)

LOCAL_SRCS	= $(LOCAL_OBJS:.obj=.cpp)
LOCAL_SRCS	= $(LOCAL_SRCS:x86\=)
LOCAL_SRCS	= $(LOCAL_SRCS:x64\=)
LOCAL_SRCS	= $(LOCAL_SRCS:arm\=)
LOCAL_SRCS	= $(LOCAL_SRCS:Retail\=.\)
LOCAL_SRCS	= $(LOCAL_SRCS:Debug\=.\)

SRCS		= $(SRCS) \
		  $(LOCAL_SRCS) \
		  ..\CommunicatorF.cpp \
		  ..\ConnectionF.cpp \
		  ..\EndpointF.cpp \
		  ..\ImplicitContextF.cpp \
		  ..\LocatorF.cpp \
		  ..\LoggerF.cpp \
		  ..\ObjectAdapterF.cpp \
		  ..\ObjectFactoryF.cpp \
		  ..\PluginF.cpp \
		  ..\ProcessF.cpp \
		  ..\PropertiesF.cpp \
		  ..\RouterF.cpp \
		  ..\ServantLocatorF.cpp

HDIR		= $(headerdir)\Ice
SDIR		= $(slicedir)\Ice

PDBNAME			= $(LIBNAME:.lib=.pdb)
CPPFLAGS		= /Fd$(PDBNAME) -I. -I..\.. -DICE_BUILDING_ICE -DICE_BUILDING_ICESSL -DWIN32_LEAN_AND_MEAN $(CPPFLAGS)
SLICE2CPPFLAGS		= --ice $(SLICE2CPPFLAGS)
CORE_SLICE2CPPFLAGS	= --include-dir Ice $(SLICE2CPPFLAGS)
SSL_SLICE2CPPFLAGS 	= --include-dir IceSSL $(SLICE2CPPFLAGS)

!include $(top_srcdir)\config\Make.rules.mak

$(LIBNAME): $(LOCAL_OBJS) $(OBJS) sdks
	$(AR) $(ARFLAGS) $(OBJS) $(LOCAL_OBJS) /out:$(LIBNAME)

.cpp.d:
	@if not exist "$(ARCH)\$(CONFIG)" $(MKDIR) $(ARCH)\$(CONFIG)
	@echo Generating dependencies for $<
	@$(CXX) /E /Fo$(ARCH)\$(CONFIG)\ $(CPPFLAGS) $(CXXFLAGS) /showIncludes $< 1>$(*F).i 2>$(*F).d && \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend.vbs $(*F).cpp $(top_srcdir)
	@del /q $(*F).d $(*F).i

.cpp{$(ARCH)\$(CONFIG)\}.obj::
	@if not exist "$(ARCH)\$(CONFIG)" $(MKDIR) $(ARCH)\$(CONFIG)
	$(CXX) /c /Fo$(ARCH)\$(CONFIG)\ $(CPPFLAGS) $(CXXFLAGS) $<

.cpp{$(ARCH)\$(CONFIG)\IceDiscovery\}.obj::
	@if not exist "$(ARCH)\$(CONFIG)\IceDiscovery" $(MKDIR) $(ARCH)\$(CONFIG)\IceDiscovery
	$(CXX) /c /Fo$(ARCH)\$(CONFIG)\IceDiscovery\ $(CPPFLAGS) $(CXXFLAGS) $<

.cpp{$(ARCH)\$(CONFIG)\IceLocatorDiscovery\}.obj::
	@if not exist "$(ARCH)\$(CONFIG)\IceLocatorDiscovery" $(MKDIR) $(ARCH)\$(CONFIG)\IceLocatorDiscovery
	$(CXX) /c /Fo$(ARCH)\$(CONFIG)\IceLocatorDiscovery\ $(CPPFLAGS) $(CXXFLAGS) $<

{..\..\IceDiscovery\}.cpp{$(ARCH)\$(CONFIG)\IceDiscovery\}.obj::
	@if not exist "$(ARCH)\$(CONFIG)\IceDiscovery" $(MKDIR) $(ARCH)\$(CONFIG)\IceDiscovery
	$(CXX) /c /Fo$(ARCH)\$(CONFIG)\IceDiscovery\ $(CPPFLAGS) $(CXXFLAGS) $<

{..\..\IceLocatorDiscovery\}.cpp{$(ARCH)\$(CONFIG)\IceLocatorDiscovery\}.obj::
	@if not exist "$(ARCH)\$(CONFIG)\IceLocatorDiscovery" $(MKDIR) $(ARCH)\$(CONFIG)\IceLocatorDiscovery
	$(CXX) /c /Fo$(ARCH)\$(CONFIG)\IceLocatorDiscovery\ $(CPPFLAGS) $(CXXFLAGS) $<

.cpp{$(DEPEND_DIR)\IceDiscovery\}.d:
	@echo Generating dependencies for $<
	@$(CXX) /E $(CPPFLAGS) $(CXXFLAGS) /showIncludes $< 1>$(*F).i 2>IceDiscovery\$(*F).d && \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend.vbs $(*F).cpp $(top_srcdir)
	@del /q $(*F).d $(*F).i

.cpp{$(DEPEND_DIR)\IceLocatorDiscovery\}.d:
	@echo Generating dependencies for $<
	@$(CXX) /E $(CPPFLAGS) $(CXXFLAGS) /showIncludes $< 1>$(*F).i 2>IceLocatorDiscovery\$(*F).d && \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend.vbs $(*F).cpp $(top_srcdir)
	@del /q $(*F).d $(*F).i

{..\..\IceDiscovery\}.cpp{$(DEPEND_DIR)\IceDiscovery\}.d:
	@if not exist "$(ARCH)\$(CONFIG)" $(MKDIR) $(ARCH)\$(CONFIG)
	@echo Generating dependencies for $<
	@$(CXX) /E /Fo$(ARCH)\$(CONFIG)\ $(CPPFLAGS) $(CXXFLAGS) /showIncludes $< 1>$(*F).i 2>IceDiscovery\$(*F).d && \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend.vbs $< $(top_srcdir)
	@del /q $(*F).d $(*F).i

{..\..\IceLocatorDiscovery\}.cpp{$(DEPEND_DIR)\IceLocatorDiscovery\}.d:
	@if not exist "$(ARCH)\$(CONFIG)" $(MKDIR) $(ARCH)\$(CONFIG)
	@echo Generating dependencies for $<
	@$(CXX) /E /Fo$(ARCH)\$(CONFIG)\ $(CPPFLAGS) $(CXXFLAGS) /showIncludes $< 1>$(*F).i 2>IceLocatorDiscovery\$(*F).d && \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend.vbs $< $(top_srcdir)
	@del /q $(*F).d $(*F).i

{$(slicedir)\Ice\}.ice{Ice\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2CPP)" $(CORE_SLICE2CPPFLAGS) --depend $< | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice "..\"

{$(slicedir)\Ice}.ice.cpp:
	del /q $(headerdir)\Ice\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(CORE_SLICE2CPPFLAGS) $<
	move $(*F).h $(headerdir)\Ice
	move $(*F).cpp ..

{$(slicedir)\Ice}.ice.h:
	del /q $(headerdir)\Ice\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(CORE_SLICE2CPPFLAGS) $<
	move $(*F).h $(headerdir)\Ice
	move $(*F).cpp ..

{$(slicedir)\IceSSL\}.ice{IceSSL\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2CPP)" $(CORE_SLICE2CPPFLAGS) --depend $< | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice

{$(slicedir)\IceSSL}.ice.cpp:
	del /q $(headerdir)\IceSSL\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SSL_SLICE2CPPFLAGS) $<
	move $(*F).h $(headerdir)\IceSSL

{$(slicedir)\IceSSL}.ice.h:
	del /q $(headerdir)\IceSSL\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" $(SSL_SLICE2CPPFLAGS) $<
	move $(*F).h $(headerdir)\IceSSL

{$(slicedir)\IceDiscovery\}.ice{IceDiscovery\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2CPP)" --include-dir IceDiscovery $(SLICE2CPPFLAGS) --depend $< | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice "..\"

{$(slicedir)\IceDiscovery}.ice.cpp:
	@if not exist "IceDiscovery" $(MKDIR) IceDiscovery
	del /q IceDiscovery\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" --include-dir IceDiscovery $(SLICE2CPPFLAGS) $<
	move $(*F).h IceDiscovery

{$(slicedir)\IceDiscovery}.ice{IceDiscovery}.h:
	@if not exist "IceDiscovery" $(MKDIR) IceDiscovery
	del /q IceDiscovery\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" --include-dir IceDiscovery $(SLICE2CPPFLAGS) $<
	move $(*F).h IceDiscovery

{$(slicedir)\IceLocatorDiscovery\}.ice{IceLocatorDiscovery\}.d:
	@echo Generating dependencies for $<
	@"$(SLICE2CPP)" --include-dir IceLocatorDiscovery $(SLICE2CPPFLAGS) --depend $< | \
	cscript /NoLogo $(top_srcdir)\..\config\makedepend-slice.vbs $(*F).ice "..\"

{$(slicedir)\IceLocatorDiscovery}.ice.cpp:
	@if not exist "IceLocatorDiscovery" $(MKDIR) IceLocatorDiscovery
	del /q IceLocatorDiscovery\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" --include-dir IceLocatorDiscovery $(SLICE2CPPFLAGS) $<
	move $(*F).h IceLocatorDiscovery

{$(slicedir)\IceLocatorDiscovery}.ice{IceLocatorDiscovery}.h:
	@if not exist "IceLocatorDiscovery" $(MKDIR) IceLocatorDiscovery
	del /q IceLocatorDiscovery\$(*F).h $(*F).cpp
	"$(SLICE2CPP)" --include-dir IceLocatorDiscovery $(SLICE2CPPFLAGS) $<
	move $(*F).h IceLocatorDiscovery

clean::
	-del /q $(SOURCE_DIR)\BuiltinSequences.cpp $(HDIR)\BuiltinSequences.h
	-del /q $(SOURCE_DIR)\CommunicatorF.cpp $(HDIR)\CommunicatorF.h
	-del /q $(SOURCE_DIR)\Communicator.cpp $(HDIR)\Communicator.h
	-del /q $(SOURCE_DIR)\ConnectionF.cpp $(HDIR)\ConnectionF.h
	-del /q $(SOURCE_DIR)\Connection.cpp $(HDIR)\Connection.h
	-del /q $(SOURCE_DIR)\Current.cpp $(HDIR)\Current.h
	-del /q $(SOURCE_DIR)\Endpoint.cpp $(HDIR)\Endpoint.h
	-del /q $(SOURCE_DIR)\EndpointF.cpp $(HDIR)\EndpointF.h
	-del /q $(SOURCE_DIR)\EndpointTypes.cpp $(HDIR)\EndpointTypes.h
	-del /q $(SOURCE_DIR)\FacetMap.cpp $(HDIR)\FacetMap.h
	-del /q $(SOURCE_DIR)\ImplicitContextF.cpp $(HDIR)\ImplicitContextF.h
	-del /q $(SOURCE_DIR)\ImplicitContext.cpp $(HDIR)\ImplicitContext.h
	-del /q $(SOURCE_DIR)\Identity.cpp $(HDIR)\Identity.h
	-del /q $(SOURCE_DIR)\LocalException.cpp $(HDIR)\LocalException.h
	-del /q $(SOURCE_DIR)\LocatorF.cpp $(HDIR)\LocatorF.h
	-del /q $(SOURCE_DIR)\Locator.cpp $(HDIR)\Locator.h
	-del /q $(SOURCE_DIR)\LoggerF.cpp $(HDIR)\LoggerF.h
	-del /q $(SOURCE_DIR)\Logger.cpp $(HDIR)\Logger.h
	-del /q $(SOURCE_DIR)\Metrics.cpp $(HDIR)\Metrics.h
	-del /q $(SOURCE_DIR)\Observer.cpp $(HDIR)\Observer.h
	-del /q $(SOURCE_DIR)\ObserverF.cpp $(HDIR)\ObserverF.h
	-del /q $(SOURCE_DIR)\ObjectAdapterF.cpp $(HDIR)\ObjectAdapterF.h
	-del /q $(SOURCE_DIR)\ObjectAdapter.cpp $(HDIR)\ObjectAdapter.h
	-del /q $(SOURCE_DIR)\ObjectFactoryF.cpp $(HDIR)\ObjectFactoryF.h
	-del /q $(SOURCE_DIR)\ObjectFactory.cpp $(HDIR)\ObjectFactory.h
	-del /q $(SOURCE_DIR)\PluginF.cpp $(HDIR)\PluginF.h
	-del /q $(SOURCE_DIR)\Plugin.cpp $(HDIR)\Plugin.h
	-del /q $(SOURCE_DIR)\ProcessF.cpp $(HDIR)\ProcessF.h
	-del /q $(SOURCE_DIR)\Process.cpp $(HDIR)\Process.h
	-del /q $(SOURCE_DIR)\PropertiesF.cpp $(HDIR)\PropertiesF.h
	-del /q $(SOURCE_DIR)\Properties.cpp $(HDIR)\Properties.h
	-del /q $(SOURCE_DIR)\RemoteLogger.cpp $(HDIR)\RemoteLogger.h
	-del /q $(SOURCE_DIR)\RouterF.cpp $(HDIR)\RouterF.h
	-del /q $(SOURCE_DIR)\Router.cpp $(HDIR)\Router.h
	-del /q $(SOURCE_DIR)\ServantLocatorF.cpp $(HDIR)\ServantLocatorF.h
	-del /q $(SOURCE_DIR)\ServantLocator.cpp $(HDIR)\ServantLocator.h
	-del /q $(SOURCE_DIR)\SliceChecksumDict.cpp $(HDIR)\SliceChecksumDict.h
	-del /q $(SOURCE_DIR)\Version.cpp $(HDIR)\Version.h
	-del /q EndpointInfo.cpp $(headerdir)\IceSSL\EndpointInfo.h
	-del /q ConnectionInfo.cpp $(headerdir)\IceSSL\ConnectionInfo.h
	-del /q $(ARCH)\$(CONFIG)\*.obj
	-del /q $(ARCH)\$(CONFIG)\IceDiscovery\*.obj
	-del /q $(ARCH)\$(CONFIG)\IceLocatorDiscovery\*.obj
	-del /q $(PDBNAME)

install:: all
