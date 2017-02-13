# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

CLIENT		= $(call mktestname,client)
SERVER		= $(call mktestname,server)
SERVERAMD	= $(call mktestname,serveramd)
COLLOCATED	= $(call mktestname,collocated)

TARGETS		= $(CLIENT) $(SERVER) $(SERVERAMD) $(COLLOCATED)

SLICE_OBJS	= Test.o TestAMD.o

COBJS		= Test.o \
		  Client.o \
		  AllTests.o \
		  Twoways.o \
		  Oneways.o \
		  TwowaysAMI.o \
		  OnewaysAMI.o \
		  BatchOneways.o \
		  BatchOnewaysAMI.o

SOBJS		= Test.o \
		  TestI.o \
		  Server.o \

SAMDOBJS	= TestAMD.o \
		  TestAMDI.o \
		  ServerAMD.o

COLOBJS		= Test.o \
		  TestI.o \
		  Collocated.o \
		  AllTests.o \
		  Twoways.o \
		  Oneways.o \
		  TwowaysAMI.o \
		  OnewaysAMI.o \
		  BatchOneways.o \
		  BatchOnewaysAMI.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SAMDOBJS) \
		  $(COLOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS       := -I. -I../../include $(CPPFLAGS)

ifeq ($(XLC_COMPILER),yes)
    CPPFLAGS   := -qsuppress="1540-0895" $(CPPFLAGS)
endif


$(CLIENT): $(COBJS)
	rm -f $@
	$(call mktest,$@,$(COBJS),$(LIBS))

$(SERVER): $(SOBJS)
	rm -f $@
	$(call mktest,$@,$(SOBJS),$(LIBS))

$(SERVERAMD): $(SAMDOBJS)
	rm -f $@
	$(call mktest,$@,$(SAMDOBJS),$(LIBS))

$(COLLOCATED): $(COLOBJS)
	rm -f $@
	$(call mktest,$@,$(COLOBJS),$(LIBS))
