# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

LIBFILENAME     = $(call mklibfilename,HelloService)
SONAME          = $(call mksoname,HelloService)

TARGETS		= $(CLIENT) $(LIBFILENAME) $(SERVER)

OBJS		= Hello.o

COBJS		= Client.o

SVC_OBJS	= HelloI.o \
		  HelloServiceI.o

SVR_OBJS	= HelloI.o \
		  Server.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SVC_OBJS:.o=.cpp) \
		  $(SVR_OBJS:.o=.cpp)

SLICE_SRCS	= Hello.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)
LINKWITH	:= -lIcePack -lIceBox -lIce -lIceUtil

$(LIBFILENAME): $(OBJS) $(SVC_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS) $(SVC_OBJS),$(LINKWITH))

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) -lIcePack

$(SERVER): $(OBJS) $(SVR_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SVR_OBJS) $(LIBS) -lIcePack

include .depend
