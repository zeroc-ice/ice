# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

ROUTER		= $(bindir)/glacier2router
TARGETS         = $(ROUTER)

SLICE_OBJS      = Instrumentation.o

OBJS		= Blobject.o \
		  ClientBlobject.o \
		  FilterI.o \
		  FilterManager.o \
		  Glacier2Router.o \
		  Instance.o \
		  InstrumentationI.o \
		  ProxyVerifier.o \
		  RequestQueue.o \
		  RouterI.o \
		  RoutingTable.o \
		  ServerBlobject.o \
		  SessionRouterI.o \
                  $(SLICE_OBJS)

RPATH_DIR	= $(LOADER_PATH)/../$(libsubdir)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS)
SLICE2CPPFLAGS	:= --include-dir Glacier2 $(SLICE2CPPFLAGS)

$(ROUTER): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(OBJS) -lGlacier2 $(LIBS) -lIceSSL $(OPENSSL_RPATH_LINK)

install:: all
	$(call installprogram,$(ROUTER),$(DESTDIR)$(install_bindir))
	$(call installdata,$(top_srcdir)/../man/man1/glacier2router.1,$(DESTDIR)$(install_mandir))
