# **********************************************************************
#
# Copyright (c) 2003 - 2004
# ZeroC, Inc.
# North Palm Beach, FL, USA
#
# All Rights Reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


top_srcdir	= ../../..

CLIENT		= client
SERVER		= server

LIBFILENAME     = $(call mklibfilename,HelloService,$(VERSION))
SONAME          = $(call mksoname,HelloService,$(SOVERSION))
LIBNAME         = $(call mklibname,HelloService)

TARGETS		= $(CLIENT) $(LIBFILENAME) $(SONAME) $(LIBNAME) $(SERVER)

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

$(SONAME): $(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(LIBNAME): $(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) -lIcePack

$(SERVER): $(OBJS) $(SVR_OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SVR_OBJS) $(LIBS) -lIcePack

include .depend
