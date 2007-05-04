# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceE,$(VERSION))
SONAME          = $(call mksoname,IceE,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceE)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

TRANSPORT_DIR	= $(top_srcdir)/src/TcpTransport

%.o: $(TRANSPORT_DIR)/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

TRANSPORT_OBJS	= Acceptor.o \
		  Connector.o \
		  EndpointFactory.o \
		  TcpEndpoint.o \
		  Transceiver.o

LOCAL_OBJS	= BasicStream.o \
		  Buffer.o \
		  BuiltinSequences.o \
		  Communicator.o \
		  Cond.o \
		  Connection.o \
		  ConvertUTF.o \
		  Current.o \
		  DefaultsAndOverrides.o \
		  Endpoint.o \
		  ExceptionBase.o \
		  FactoryTable.o \
		  FactoryTableDef.o \
		  Identity.o \
		  Incoming.o \
		  IncomingConnectionFactory.o \
		  Initialize.o \
		  Instance.o \
		  LocalException.o \
		  Locator.o \
		  LocatorInfo.o \
		  Logger.o \
		  LoggerI.o \
		  LoggerUtil.o \
		  Network.o \
		  Object.o \
		  ObjectAdapter.o \
		  ObjectAdapterFactory.o \
		  OperationMode.o \
		  Outgoing.o \
		  OutgoingConnectionFactory.o \
		  Properties.o \
		  Protocol.o \
		  Proxy.o \
		  ProxyFactory.o \
		  RecMutex.o \
		  Reference.o \
		  ReferenceFactory.o \
		  Router.o \
		  RouterInfo.o \
		  SafeStdio.o \
		  ServantManager.o \
		  Shared.o \
		  StaticMutex.o \
		  StringConverter.o \
		  StringUtil.o \
		  Thread.o \
		  ThreadException.o \
		  Time.o \
		  TraceLevels.o \
		  TraceUtil.o \
		  UnknownEndpoint.o \
		  Unicode.o \
		  UUID.o

OBJS		= $(LOCAL_OBJS) $(TRANSPORT_OBJS)

SRCS		= $(LOCAL_OBJS:.o=.cpp) \
		  $(patsubst %.o,$(TRANSPORT_DIR)/%.cpp,$(TRANSPORT_OBJS)) 

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/LocatorF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Router.ice

HDIR		= $(includedir)/IceE
SDIR		= $(slicedir)/IceE

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_API_EXPORTS

SLICE2CPPEFLAGS	:= --ice --include-dir IceE --dll-export ICE_API $(SLICE2CPPEFLAGS)

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
