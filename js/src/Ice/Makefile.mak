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

GEN_SRCS = BuiltinSequences.js \
	   Connection.js \
	   ConnectionF.js \
	   Current.js \
	   Endpoint.js \
	   EndpointF.js \
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
	Address.js \
	ArrayUtil.js \
	AsyncResultBase.js \
	AsyncResult.js \
	AsyncStatus.js \
	Base64.js \
	BasicStream.js \
	BatchOutgoingAsync.js \
	Class.js \
	Communicator.js \
	ConnectionBatchOutgoingAsync.js \
	ConnectionI.js \
	ConnectionMonitor.js \
	ConnectionReaper.js \
	ConnectionRequestHandler.js \
	ConnectRequestHandler.js \
	DefaultsAndOverrides.js \
	DispatchStatus.js \
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
	LocalExceptionWrapper.js \
	LocatorInfo.js \
	LocatorManager.js \
	LocatorTable.js \
	Logger.js \
	Long.js \
	ObjectAdapterFactory.js \
	ObjectAdapterI.js \
	ObjectFactory.js \
	ObjectFactoryManager.js \
	Object.js \
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
	ProxyBatchOutgoingAsync.js \
	ProxyFactory.js \
	Reference.js \
	ReferenceMode.js \
	RetryQueue.js \
	RouterInfo.js \
	RouterManager.js \
	ServantManager.js \
	SocketOperation.js \
	StreamHelpers.js \
	StringUtil.js \
	Struct.js \
	Timer.js \
	TraceLevels.js \
	TraceUtil.js \
	CompactIdRegistry.js \
	UnknownSlicedObject.js \
	UUID.js

NODEJS_SRCS = \
	Buffer.js \
	Ice.js \
	TcpEndpointFactory.js \
	TcpEndpointI.js \
	TcpTransceiver.js \

BROWSER_SRCS = \
	browser\Buffer.js \
	browser\EndpointInfo.js \
	browser\ConnectionInfo.js \
	browser\Transceiver.js \
	browser\EndpointI.js \
	browser\EndpointFactory.js

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
	

SLICE2JSFLAGS	= $(SLICE2JSFLAGS) --ice -I"$(slicedir)"

# IceWS slice files
browser\EndpointInfo.js: "$(SLICE2JS)" "$(SLICEPARSERLIB)"
	del /q browser\EndpointInfo.js
	"$(SLICE2JS)" $(SLICE2JSFLAGS) "$(slicedir)\IceWS\EndpointInfo.ice"
	move EndpointInfo.js browser

browser\ConnectionInfo.js: "$(SLICE2JS)" "$(SLICEPARSERLIB)"
	-del /q browser\ConnectionInfo.js
	"$(SLICE2JS)" $(SLICE2JSFLAGS) "$(slicedir)\IceWS\ConnectionInfo.ice"
	move ConnectionInfo.js browser

MODULEDIR	= $(install_moduledir)\$(LIBNAME)

install:: all
	@if not exist $(MODULEDIR) \
	    @echo "Creating $(MODULEDIR)" && \
	    mkdir "$(MODULEDIR)"
	@for %i in ( $(INSTALL_SRCS) ) do \
	    copy %i "$(MODULEDIR)"
	copy package.json "$(MODULEDIR)"

