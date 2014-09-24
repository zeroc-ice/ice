# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME = Ice

MODULES = $(LIBNAME) IceMX IceSSL

GEN_SRCS = \
	BuiltinSequences.js \
	Connection.js \
	ConnectionF.js \
	Current.js \
	Endpoint.js \
	EndpointF.js \
	EndpointInfo.js \
	EndpointI.js \
	EndpointTypes.js \
	Identity.js \
	LocalException.js \
	Locator.js \
	Metrics.js \
	ObjectAdapterF.js \
	Process.js \
	ProcessF.js \
	PropertiesAdmin.js \
	Router.js \
	SliceChecksumDict.js \
	Version.js

COMMON_SRCS = \
	ACM.js \
	Address.js \
	ArrayUtil.js \
	AsyncResult.js \
	AsyncResultBase.js \
	AsyncStatus.js \
	Base64.js \
	BasicStream.js \
	Class.js \
	Communicator.js \
	CompactIdRegistry.js \
	ConnectionI.js \
	ConnectionRequestHandler.js \
	ConnectRequestHandler.js \
	DefaultsAndOverrides.js \
	DispatchStatus.js \
	EndpointI.js \
	EndpointFactoryManager.js \
	EnumBase.js \
	Exception.js \
	ExUtil.js \
	FormatType.js \
	HashMap.js \
	HashUtil.js \
	IdentityUtil.js \
	ImplicitContextI.js \
	IncomingAsync.js \
	Initialize.js \
	Instance.js \
	IPEndpointI.js \
	LocatorInfo.js \
	LocatorManager.js \
	LocatorTable.js \
	Logger.js \
	Long.js \
	Object.js \
	ObjectAdapterFactory.js \
	ObjectAdapterI.js \
	ObjectFactory.js \
	ObjectFactoryManager.js \
	ObjectPrx.js \
	OpaqueEndpointI.js \
	Operation.js \
	OptionalFormat.js \
	OutgoingAsync.js \
	OutgoingConnectionFactory.js \
	ProcessLogger.js \
	Promise.js \
	Properties.js \
	Property.js \
	PropertyNames.js \
	Protocol.js \
	ProtocolInstance.js \
	ProxyFactory.js \
	Reference.js \
	ReferenceMode.js \
	RetryException.js \
	RetryQueue.js \
	RouterInfo.js \
	RouterManager.js \
	ServantManager.js \
	SocketOperation.js \
	StreamHelpers.js \
	StringUtil.js \
	Struct.js \
	TcpEndpointFactory.js \
	TcpEndpointI.js \
	Timer.js \
	TraceLevels.js \
	TraceUtil.js \
	UnknownSlicedObject.js \
	UUID.js \
	WSEndpoint.js \
	WSEndpointFactory.js

NODEJS_SRCS = \
	Buffer.js \
	Ice.js \
	ModuleRegistry.js \
	TcpTransceiver.js

BROWSER_SRCS = \
	browser\Buffer.js \
	browser\ModuleRegistry.js \
	browser\WSTransceiver.js

!if "$(OPTIMIZE)" != "yes"
NODEJS_SRCS	= $(NODEJS_SRCS) Debug.js
BROWSER_SRCS	= $(BROWSER_SRCS) browser\Debug.js
!endif

SDIR		= $(slicedir)\Ice

SRCS		= $(BROWSER_SRCS) $(GEN_SRCS) $(COMMON_SRCS)
INSTALL_SRCS	= $(NODEJS_SRCS) $(GEN_SRCS) $(COMMON_SRCS)

!include $(top_srcdir)\config\Make.rules.mak.js

# Prevent generation of these files from .ice files
Communicator.js:
	
Properties.js:
	
Logger.js:
	
ServantLocator.js:
	
ObjectFactory.js:
	
SLICE2JSFLAGS	= $(SLICE2JSFLAGS) --ice -I"$(slicedir)" --icejs

MODULEDIR	= $(install_moduledir)\$(LIBNAME)

# NOTE: we include IceSSL generated code in the Ice.js module to allow
# parsing SSL endpoints.
EndpointInfo.js : "$(slicedir)\IceSSL\EndpointInfo.ice"
        "$(SLICE2JS)" $(SLICE2JSFLAGS) "$(slicedir)\IceSSL\EndpointInfo.ice"

install:: all
	@if not exist $(MODULEDIR) \
	    @echo "Creating $(MODULEDIR)" && \
	    mkdir "$(MODULEDIR)"
	@for %i in ( $(INSTALL_SRCS) ) do \
	    copy %i "$(MODULEDIR)"

