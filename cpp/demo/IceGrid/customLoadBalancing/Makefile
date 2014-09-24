# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

LIBFILENAME     = $(call mklibfilename,RegistryPlugin,$(VERSION))
SONAME		= $(call mksoname,RegistryPlugin,$(SOVERSION)) 
LIBNAME		= $(call mklibname,RegistryPlugin)

TARGETS		= $(CLIENT) $(SERVER) $(call mklibtargets,$(LIBFILENAME),$(SONAME),$(LIBNAME))

SLICE_OBJS	= Pricing.o

COBJS		= $(SLICE_OBJS) \
		  Client.o

SOBJS		= $(SLICE_OBJS) \
		  PricingI.o \
		  Server.o

POBJS		= RegistryPlugin.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(POBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(COBJS) -lIceGrid -lGlacier2 $(LINKWITH)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(SOBJS) $(LINKWITH)

$(LIBFILENAME): $(POBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(POBJS),-lIceGrid -lGlacier2 $(LINKWITH))

$(SONAME): $(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(LIBNAME): $(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

clean::
	-rm -rf db/node/* db/registry/*
