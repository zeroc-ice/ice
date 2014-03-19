# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME = Ice

TARGETS	= $(call mklibtargets,$(LIBNAME))

SLICES = $(SDIR)/BuiltinSequences.ice \
	$(SDIR)/Connection.ice \
	$(SDIR)/ConnectionF.ice \
	$(SDIR)/Current.ice \
	$(SDIR)/Endpoint.ice \
	$(SDIR)/EndpointF.ice \
	$(SDIR)/EndpointTypes.ice \
	$(SDIR)/Identity.ice \
	$(SDIR)/LocalException.ice \
	$(SDIR)/Locator.ice \
	$(SDIR)/Metrics.ice \
	$(SDIR)/ObjectAdapterF.ice \
	$(SDIR)/Process.ice \
	$(SDIR)/ProcessF.ice \
	$(SDIR)/PropertiesAdmin.ice \
	$(SDIR)/Router.ice \
	$(SDIR)/SliceChecksumDict.ice \
	$(SDIR)/Version.ice

SDIR	= $(slicedir)/Ice

GEN_SRCS = $(patsubst $(SDIR)/%.ice, %.js, $(SLICES))

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
	browser/Buffer.js \
	browser/EndpointInfo.js \
	browser/ConnectionInfo.js \
	browser/Transceiver.js \
	browser/EndpointI.js \
	browser/EndpointFactory.js

ifneq ($(OPTIMIZE),yes)
    NODEJS_SRCS := $(NODEJS_SRCS) Debug.js
    BROWSER_SRCS := $(BROWSER_SRCS) browser/Debug.js
endif

SRCS		:= $(BROWSER_SRCS) $(GEN_SRCS) $(COMMON_SRCS)
INSTALL_SRCS	:= $(NODEJS_SRCS) $(GEN_SRCS) $(COMMON_SRCS)

include $(top_srcdir)/config/Make.rules.js

# Prevent generation of these files from .ice files
Communicator.js:
	
Properties.js:
	
Logger.js:
	
ServantLocator.js:
	
ObjectFactory.js:
	

SLICE2JSFLAGS	:= $(SLICE2JSFLAGS) --ice -I$(slicedir)

# IceWS slice files
browser/%.js: $(slicedir)/IceWS/%.ice $(SLICE2JS) $(SLICEPARSERLIB)
	rm -f browser/$(*F).js
	$(SLICE2JS) $(SLICE2JSFLAGS) $<
	mv $(*F).js browser/

lint:: $(SRCS)
	jshint $(LINTFLAGS) $(NODEJS_SRCS) $(BROWSER_SRCS) $(GEN_SRCS) $(COMMON_SRCS)

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBNAME))
	$(call installmodule,$(DESTDIR)$(install_moduledir),$(INSTALL_SRCS),$(LIBNAME))

