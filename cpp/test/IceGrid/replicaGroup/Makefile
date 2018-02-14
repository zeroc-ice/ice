# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

SVCFILENAME     = $(call mklibfilename,TestService)
SVCSONAME	= $(call mksoname,TestService) 

PLUGINFILENAME  = $(call mklibfilename,RegistryPlugin,$(VERSION))
PLUGINSONAME	= $(call mksoname,RegistryPlugin,$(SOVERSION)) 

TARGETS		= $(CLIENT) $(SERVER) $(SVCFILENAME) $(call mklibtargets,$(PLUGINFILENAME),$(PLUGINSONAME))

SLICE_OBJS	= Test.o

COBJS		= $(SLICE_OBJS) \
		  Client.o \
		  AllTests.o

SOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Server.o

POBJS		= RegistryPlugin.o

SERVICE_OBJS	= $(SLICE_OBJS) \
		  TestI.o \
		  Service.o

OBJS		= $(COBJS) \
		  $(POBJS) \
		  $(SOBJS) \
		  $(SERVICE_OBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) -lIceGrid -lGlacier2 -lIceBox $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(LIBS)

$(SVCFILENAME): $(SERVICE_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SVCSONAME),$(SERVICE_OBJS),-lIceBox $(LINKWITH))

$(PLUGINFILENAME): $(POBJS)
	rm -f $@
	$(call mkshlib,$@,$(PLUGINSONAME),$(POBJS),-lIceGrid -lGlacier2 $(LINKWITH))

$(PLUGINSONAME): $(PLUGINFILENAME)
	rm -f $@
	ln -s $(PLUGINFILENAME) $@

clean::
	-rm -f build.txt
	-rm -rf db/node db/registry db/replica-*
