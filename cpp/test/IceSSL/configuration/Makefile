# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)
SERVER		= $(call mktestname,server)

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= Test.o

COBJS		= $(SLICE_OBJS) \
		  Client.o \
		  AllTests.o

SOBJS		= $(SLICE_OBJS) \
		  TestI.o \
		  Server.o

OBJS		= $(COBJS) \
		  $(SOBJS)

GENPIC          = no

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS) 

ifeq ($(UNAME),Darwin)
LINK_WITH = -framework Security -framework CoreFoundation
else
CPPFLAGS	+= $(OPENSSL_FLAGS)
endif

ifeq ($(UNAME),MINGW)
LINK_WITH = $(SSL_OS_LIBS)
endif

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(ICESSL_LIBS) $(LINK_WITH) $(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(ICESSL_LIBS) $(LINK_WITH) $(LIBS))
