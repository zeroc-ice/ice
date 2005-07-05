# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceCE,$(VERSION))
SONAME          = $(call mksoname,IceCE,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceCE)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

TRANSPORT_DIR	= $(top_srcdir)/src/TcpTransport

ICE_DIR	= $(top_srcdir)/src/Ice

TRANSPORT_OBJS	= Acceptor.o \
		  Connector.o \
		  Endpoint.o \
		  EndpointFactory.o \
		  Transceiver.o

ICE_OBJS	= BasicStream.o \
		  Buffer.o \
		  Communicator.o \
		  Cond.o \
		  Connection.o \
		  DefaultsAndOverrides.o \
		  Exception.o \
		  ExceptionBase.o \
		  FactoryTable.o \
		  FactoryTableDef.o \
		  IdentityUtil.o \
		  Incoming.o \
		  IncomingConnectionFactory.o \
		  Initialize.o \
		  Instance.o \
		  LocalObject.o \
		  LocatorInfo.o \
		  LoggerI.o \
		  LoggerUtil.o \
		  Network.o \
		  Object.o \
		  ObjectAdapterFactory.o \
		  ObjectAdapterI.o \
		  Outgoing.o \
		  OutgoingConnectionFactory.o \
		  PropertiesI.o \
		  PropertyNames.o \
		  Proxy.o \
		  ProxyFactory.o \
		  RecMutex.o \
		  Reference.o \
		  ReferenceFactory.o \
		  RouterInfo.o \
		  RoutingTable.o \
		  RWRecMutex.o \
		  SafeStdio.o \
		  ServantManager.o \
		  Shared.o \
		  StaticMutex.o \
		  StringUtil.o \
		  Thread.o \
		  ThreadException.o \
		  Time.o \
		  TraceLevels.o \
		  TraceUtil.o \
		  UUID.o

LOCAL_OBJS	= BuiltinSequences.o \
		  Current.o \
		  FacetMap.o \
		  Identity.o \
		  LocalException.o \
		  Locator.o \
		  LocatorF.o \
		  Logger.o \
		  LoggerF.o \
		  ObjectAdapter.o \
		  ObjectAdapterF.o \
		  Properties.o \
		  PropertiesF.o \
		  Router.o \
		  RouterF.o

OBJS		= $(LOCAL_OBJS) $(ICE_OBJS) $(TRANSPORT_OBJS)

TRANSPORT_SRCS	= $(TRANSPORT_DIR)/Acceptor.cpp \
		  $(TRANSPORT_DIR)/Connector.cpp \
		  $(TRANSPORT_DIR)/Endpoint.cpp \
		  $(TRANSPORT_DIR)/EndpointFactory.cpp \
		  $(TRANSPORT_DIR)/Transceiver.cpp

SRCS		= $(LOCAL_OBJS:.o=.cpp) $(TRANSPORT_SRCS)

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/Current.ice \
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/LocalException.ice \
		  $(SDIR)/LocatorF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LoggerF.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/ObjectAdapterF.ice \
		  $(SDIR)/ObjectAdapter.ice \
		  $(SDIR)/PropertiesF.ice \
		  $(SDIR)/Properties.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Router.ice

HDIR		= $(includedir)/Ice
SDIR		= $(slicedir)/Ice

include $(top_srcdir)/config/Make.rules
ifeq ($(USE_SPARC_ASM),yes)
ASRC		= 	IceAtomic.c
AOBJ		= 	IceAtomic.o
SRCS		:= 	$(SRCS) $(ASRCS)
OBJS		:= 	$(OBJS) $(AOBJ)
CFLAGS		:= 	$(CFLAGS) -fomit-frame-pointer
endif

%.o: $(TRANSPORT_DIR)/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o: $(ICE_DIR)/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DICE_PROTOCOL_API_EXPORTS
SLICE2CPPEFLAGS	:= --ice --include-dir Ice --dll-export ICE_API $(SLICE2CPPEFLAGS)

LINKWITH        := $(ICE_OS_LIBS)

ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@
endif

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend
