# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..
INCLUDE_DIR	= Ice

!include $(top_srcdir)/config/Make.rules.mak

!if "$(WINRT)" != "yes"

install::
	@if not exist "$(install_includedir)\Ice" \
	    @echo "Creating $(install_includedir)\Ice..." && \
	    mkdir "$(install_includedir)\Ice"

	@for %i in ( *.h ) do \
	    @echo Installing %i && \
	    copy %i "$(install_includedir)\Ice"

!else

SDK_HEADERS =	$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\BasicStream.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Buffer.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\BuiltinSequences.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Communicator.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\CommunicatorAsync.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\CommunicatorF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Config.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Connection.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ConnectionAsync.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ConnectionF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ConnectionFactoryF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ConnectionIF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ConnectionMonitorF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Current.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Direct.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\DispatchInterceptor.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Dispatcher.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\DynamicLibrary.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\DynamicLibraryF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Endpoint.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\EndpointF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\EndpointFactory.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\EndpointFactoryF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\EndpointIF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\EndpointTypes.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Exception.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\FacetMap.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\FactoryTable.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\FactoryTableInit.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Format.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Functional.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\GCCountMap.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\GCShared.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Handle.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Ice.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\IconvStringConverter.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Identity.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ImplicitContext.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ImplicitContextF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Incoming.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\IncomingAsync.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\IncomingAsyncF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Initialize.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\InstanceF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\LocalException.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\LocalObject.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\LocalObjectF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Locator.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\LocatorF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Logger.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\LoggerF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\LoggerUtil.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Metrics.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Object.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ObjectAdapter.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ObjectAdapterF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ObjectAdapterFactoryF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ObjectF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ObjectFactory.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ObjectFactoryF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Observer.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ObserverF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ObserverHelper.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Outgoing.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\OutgoingAsync.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\OutgoingAsyncF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Plugin.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\PluginF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Process.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ProcessF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Properties.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\PropertiesF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Protocol.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ProtocolPluginFacade.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ProtocolPluginFacadeF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Proxy.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ProxyF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ProxyFactoryF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ProxyHandle.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ReferenceF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\RequestHandlerF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Router.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\RouterF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ServantLocator.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ServantLocatorF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\ServantManagerF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\SliceChecksumDict.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\SliceChecksums.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\SlicedData.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\SlicedDataF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Stats.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\StatsF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Stream.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\StreamF.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\StreamTraits.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\StringConverter.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\UndefSysMacros.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\UserExceptionFactory.h \
		$(SDK_INCLUDE_PATH)\$(INCLUDE_DIR)\Version.h

all:: $(SDK_HEADERS)
!endif
