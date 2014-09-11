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

SVCFILENAME     = $(call mklibfilename,TestService)
SVCSONAME	= $(call mksoname,TestService) 

PLUGINFILENAME  = $(call mklibfilename,RegistryPlugin,$(VERSION))
PLUGINSONAME	= $(call mksoname,RegistryPlugin,$(SOVERSION)) 

TARGETS		= $(CLIENT) $(SERVER) $(SVCFILENAME) $(call mklibtargets,$(PLUGINFILENAME),$(PLUGINSONAME))

OBJS		= Test.o

COBJS		= Client.o \
		  AllTests.o

SOBJS		= TestI.o \
		  Server.o

POBJS		= RegistryPlugin.o

SERVICE_OBJS	= TestI.o \
		  Service.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lIceGrid -lGlacier2 -lIceBox $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(SVCFILENAME): $(OBJS) $(SERVICE_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SVCSONAME),$(OBJS) $(SERVICE_OBJS),-lIceBox $(LINKWITH))

$(PLUGINFILENAME): $(POBJS)
	rm -f $@
	$(call mkshlib,$@,$(PLUGINSONAME),$(POBJS),-lIceGrid -lGlacier2 $(LINKWITH))

$(PLUGINSONAME): $(PLUGINFILENAME)
	rm -f $@
	ln -s $(PLUGINFILENAME) $@

clean::
	-rm -f build.txt
	-rm -rf db/node db/registry db/replica-*

include .depend
