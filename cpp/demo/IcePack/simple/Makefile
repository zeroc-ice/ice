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

TARGETS		= $(CLIENT) $(SERVER) $(LIBFILENAME) $(SONAME) $(LIBNAME)

OBJS		= Hello.o

COBJS		= Client.o

SOBJS		= HelloI.o \
		  Server.o

SVCOBJS		= HelloServiceI.o \
		  HelloI.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
	    	  $(SVCOBJS:.o=.cpp)

SLICE_SRCS	= Hello.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)
LINKWITH	:= -lIcePack -lIceBox -lIce -lIceUtil

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS) -lIcePack

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(LIBS)

$(LIBFILENAME): $(OBJS) $(SVCOBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS) $(SVCOBJS),$(LINKWITH))

$(SONAME): $(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(LIBNAME): $(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

include .depend
