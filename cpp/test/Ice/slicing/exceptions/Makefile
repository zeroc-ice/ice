# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

CLIENT		= $(call mktestname,client)
SERVER		= $(call mktestname,server)
SERVERAMD	= $(call mktestname,serveramd)

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD)

SLICE_OBJS	= Test.o TestAMD.o ClientPrivate.o ServerPrivate.o ServerPrivateAMD.o

COBJS		= Test.o \
		  Client.o \
		  ClientPrivate.o \
		  AllTests.o

SOBJS		= Test.o \
		  ServerPrivate.o \
		  TestI.o \
		  Server.o

SAMDOBJS	= TestAMD.o \
		  ServerPrivateAMD.o \
		  TestAMDI.o \
		  ServerAMD.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SAMDOBJS)

include $(top_srcdir)/config/Make.rules

ICECPPFLAGS	:= -I. $(ICECPPFLAGS)

CPPFLAGS	:= -I. -I../../../include $(CPPFLAGS)

LINKWITH	:= -lTestCommon $(LIBS)

$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LINKWITH))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LINKWITH))

$(SERVERAMD): $(SAMDOBJS)
	rm -f $@
	$(call mktest,$@,$(SAMDOBJS),$(LINKWITH))
