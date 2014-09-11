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

OBJS		= Pricing.o

COBJS		= Client.o

SOBJS		= PricingI.o \
		  Server.o

POBJS		= RegistryPlugin.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \

SLICE_SRCS	= Pricing.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lIceGrid -lGlacier2 $(LINKWITH)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LINKWITH)

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

include .depend
