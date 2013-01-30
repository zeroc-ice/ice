# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

ROUTER		= $(bindir)/glacier2router
TARGETS         = $(ROUTER)

OBJS		= Blobject.o \
		  ClientBlobject.o \
		  CryptPermissionsVerifierI.o \
		  Glacier2Router.o \
		  Instance.o \
		  Instrumentation.o \
		  InstrumentationI.o \
		  ProxyVerifier.o \
		  RequestQueue.o \
		  RouterI.o \
		  RoutingTable.o \
		  FilterI.o \
		  FilterManager.o \
		  ServerBlobject.o \
		  SessionRouterI.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= Instrumentation.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(OPENSSL_FLAGS)
SLICE2CPPFLAGS	:= --include-dir Glacier2 $(SLICE2CPPFLAGS)

$(ROUTER): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lGlacier2 $(LIBS) -lIceSSL $(OPENSSL_LIBS)

install:: all
	$(call installprogram,$(ROUTER),$(install_bindir))

include .depend
