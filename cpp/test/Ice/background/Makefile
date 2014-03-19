# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)
SERVER		= $(call mktestname,server)

TSPFILENAME     = $(call mklibfilename,TestTransport,$(VERSION))
TSPSONAME	= $(call mksoname,TestTransport,$(SOVERSION)) 
TSPLIBNAME	= $(call mklibname,TestTransport)

TARGETS		= $(CLIENT) $(SERVER) $(call mklibtargets,$(TSPFILENAME),$(TSPSONAME),$(TSPLIBNAME))

COBJS		= Configuration.o \
		  Test.o \
		  Client.o \
		  AllTests.o

SOBJS		= Configuration.o \
		  Test.o \
		  TestI.o \
		  Server.o

TRANSPORT_OBJS	= Test.o \
                  Configuration.o \
		  Connector.o \
		  Acceptor.o \
		  EndpointI.o \
		  Transceiver.o \
		  EndpointFactory.o \
		  PluginI.o

SRCS		= $(TRANSPORT_OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include -I../../../src $(CPPFLAGS) $(NO_DEPRECATED_FLAGS)
LINKWITH	:= $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LIBS))

$(TSPFILENAME): $(TRANSPORT_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(TSPSONAME),$(OBJS) $(TRANSPORT_OBJS),$(LINKWITH))

$(TSPSONAME): $(TSPFILENAME)
	rm -f $@
	ln -s $(TSPFILENAME) $@

$(TSPLIBNAME): $(TSPSONAME)
	rm -f $@
	ln -s $(TSPSONAME) $@

include .depend
