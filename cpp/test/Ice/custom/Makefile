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

SLICE_OBJS	= Test.o \
		  TestAMD.o \
		  Wstring.o \
		  WstringAMD.o

COBJS		= Test.o \
		  Wstring.o \
		  Client.o \
		  AllTests.o \
		  MyByteSeq.o \
		  StringConverterI.o

SOBJS		= Test.o \
		  Wstring.o \
		  TestI.o \
		  WstringI.o \
		  Server.o \
		  MyByteSeq.o \
		  StringConverterI.o

SAMDOBJS	= TestAMD.o \
		  TestAMDI.o \
		  WstringAMD.o \
		  WstringAMDI.o \
		  ServerAMD.o \
		  MyByteSeq.o \
		  StringConverterI.o

COLOBJS		= Test.o \
		  Wstring.o \
		  TestI.o \
		  WstringI.o \
		  Collocated.o \
		  AllTests.o \
		  MyByteSeq.o \
		  StringConverterI.o

OBJS		= $(COBJS) \
		  $(SOBJS) \
		  $(SAMDOBJS) \
		  $(COLOBJS)

GENPIC          = no

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I../../include $(CPPFLAGS) -DSTRING_VIEW_IGNORE_STRING_CONVERTER

#
# Disables optimization when building with Solaris CC:
# AllTests.cpp takes a very very long time to compile with -O
#
ifeq ($(UNAME),SunOS)
ifeq ($(CXX),CC)
ifeq ($(OPTIMIZE),yes)

AllTests.o:  AllTests.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -xO0 $<
endif
endif
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
