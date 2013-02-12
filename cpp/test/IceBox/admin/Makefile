# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= client

SVCFILENAME     = $(call mklibfilename,TestService)
SVCSONAME	= $(call mksoname,TestService) 

TARGETS		= $(CLIENT) $(SVCFILENAME)

OBJS		= Test.o \

COBJS		= Client.o \
		  AllTests.o

SERVICE_OBJS	= TestI.o \
		  Service.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp) \

SLICE_SRCS	= Test.ice

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS)
LINKWITH	:= -lIceBox $(BZIP2_RPATH_LINK) -lIce -lIceUtil

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SVCFILENAME): $(OBJS) $(SERVICE_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SVCSONAME),$(OBJS) $(SERVICE_OBJS), $(LINKWITH))

include .depend
