# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of IceE is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceEC,$(VERSION))
SONAME          = $(call mksoname,IceEC,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceEC)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

TRANSPORT_DIR	= $(top_srcdir)/src/TcpTransport

ICE_DIR	= $(top_srcdir)/src/IceE

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
		  ExceptionBase.o \
		  FactoryTable.o \
		  FactoryTableDef.o \
		  IdentityUtil.o \
		  Initialize.o \
		  Instance.o \
		  LocalException.o \
		  LocalObject.o \
		  LocatorInfo.o \
		  LoggerI.o \
		  LoggerUtil.o \
		  Network.o \
		  OperationMode.o \
		  Outgoing.o \
		  OutgoingConnectionFactory.o \
		  Properties.o \
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
		  FacetMap.o \
		  Identity.o \
		  Locator.o \
		  LocatorF.o \
		  Logger.o \
		  LoggerF.o \
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
		  $(SDIR)/FacetMap.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/LocatorF.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LoggerF.ice \
		  $(SDIR)/Logger.ice \
		  $(SDIR)/RouterF.ice \
		  $(SDIR)/Router.ice

HDIR		= $(includedir)/IceE
SDIR		= $(slicedir)/IceE

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

CPPFLAGS	:= -I.. $(CPPFLAGS) -DICEE_API_EXPORTS
SLICE2CPPEFLAGS	:= --ice --include-dir IceE --dll-export ICEE_API $(SLICE2CPPEFLAGS)

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
