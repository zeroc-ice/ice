# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

SVCFILENAME     = $(call mklibfilename,TestService,$(VERSION))
SVCSONAME	= $(call mksoname,TestService,$(SOVERSION)) 
SVCLIBNAME	= $(call mklibname,TestService) 

TARGETS		= $(CLIENT) $(SERVER) \
                  $(SVCFILENAME) $(SVCSONAME) $(SVCLIBNAME)

OBJS		= Test.o

COBJS		= Client.o \
		  AllTests.o

SOBJS		= TestI.o \
		  Server.o

SERVICE_OBJS	= TestI.o \
		  Service.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp)

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LINKWITH	:= -lIceBox -lIce -lIceUtil

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) -lIcePack -lIceBox $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(SVCFILENAME): $(OBJS) $(SERVICE_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SVCSONAME),$(OBJS) $(SERVICE_OBJS),$(LINKWITH))

$(SVCSONAME): $(SVCFILENAME)
	rm -f $@
	ln -s $(SVCFILENAME) $@

$(SVCLIBNAME): $(SVCSONAME)
	rm -f $@
	ln -s $(SVCSONAME) $@

clean::
	rm -f db/node/db/servers db/node/db/serveradapters db/node/db/log.*
	rm -f db/registry/*

include .depend
