# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

ROUTER		= $(top_srcdir)/bin/glacier2router
TARGETS         = $(ROUTER)

OBJS		= Blobject.o \
		  ClientBlobject.o \
		  CryptPermissionsVerifierI.o \
		  Glacier2Router.o \
		  Instance.o \
		  ProxyVerifier.o \
		  RequestQueue.o \
		  RouterI.o \
		  RoutingTable.o \
		  FilterI.o \
		  FilterManager.o \
		  ServerBlobject.o \
		  SessionRouterI.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) $(OPENSSL_FLAGS)

$(ROUTER): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lGlacier2 $(LIBS) -lIceSSL $(OPENSSL_LIBS)

install:: all
	$(call installprogram,$(ROUTER),$(install_bindir))

include .depend
