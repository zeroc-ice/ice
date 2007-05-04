# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceEC,$(VERSION))
SONAME          = $(call mksoname,IceEC,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceEC)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

TRANSPORT_DIR	= $(top_srcdir)/src/TcpTransport

ICE_DIR	= $(top_srcdir)/src/IceE

TRANSPORT_OBJS	= Connector.o \
		  EndpointFactory.o \
		  TcpEndpoint.o \
		  Transceiver.o

ICE_OBJS	= BasicStream.o \
		  Buffer.o \
                  BuiltinSequences.o \
		  Communicator.o \
		  Cond.o \
		  Connection.o \
		  ConvertUTF.o \
		  DefaultsAndOverrides.o \
		  Endpoint.o \
		  ExceptionBase.o \
		  FactoryTable.o \
		  FactoryTableDef.o \
		  Identity.o \
		  Initialize.o \
		  Instance.o \
		  LocalException.o \
		  Locator.o \
		  LocatorInfo.o \
		  Logger.o \
		  LoggerI.o \
		  LoggerUtil.o \
		  Network.o \
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
		  Unicode.o

OBJS		= $(LOCAL_OBJS) $(ICE_OBJS) $(TRANSPORT_OBJS)

SRCS		= $(patsubst %.o,$(ICE_DIR)/%.cpp,$(ICE_OBJS)) \
		  $(patsubst %.o,$(TRANSPORT_DIR)/%.cpp,$(TRANSPORT_OBJS))

SLICE_SRCS	= $(SDIR)/BuiltinSequences.ice \
		  $(SDIR)/Identity.ice \
		  $(SDIR)/Locator.ice \
		  $(SDIR)/LocatorF.ice \
		  $(SDIR)/Router.ice \
		  $(SDIR)/RouterF.ice

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

$(HDIR)/%.h $(ICE_DIR)/%.cpp: $(SDIR)/%.ice
	rm -f $(HDIR)/$(*F).h $(ICE_DIR)/$(*F).cpp
	$(SLICE2CPPE) $(SLICE2CPPEFLAGS) $<
	mv $(*F).h $(HDIR)
	mv $(*F).cpp $(ICE_DIR)

%.o: $(TRANSPORT_DIR)/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

%.o: $(ICE_DIR)/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<


CPPFLAGS	:= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DICEE_PURE_CLIENT

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
