# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

MAXWARN         = yes

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

PROC_SRCS 	= CurrentSqlContext.pc \
		  DeptFactoryI.pc \
		  DeptI.pc \
		  EmpI.pc \
		  Server.pc \
		  Util.pc

SLICE_OBJS	= HR.o

COBJS		= $(SLICE_OBJS) \
		  Client.o

SOBJS		= $(SLICE_OBJS) \
		  $(PROC_SRCS:.pc=.o)

OBJS		= $(COBJS) \
		  $(SOBJS)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. -I$(ORACLE_HOME)/precomp/public -DSQLCA_NONE $(CPPFLAGS)

GENERATED_PROC_FILES  = $(PROC_SRCS:.pc=.cpp)

ORACLE_LIBS     = -L$(ORACLE_HOME)/lib -lclntsh

.SUFFIXES:
.SUFFIXES:		.pc .cpp .c .o

#
# The rm -f tp* $*.lis is to work around a proc bug on Linux
#
.pc.cpp:
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $<
	rm -f tp* $*.lis

$(CLIENT): $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(COBJS) $(LIBS)

$(SERVER): $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) $(LDEXEFLAGS) -o $@ $(SOBJS) $(ORACLE_LIBS) $(LIBS)

clean::
	-rm -f $(GENERATED_PROC_FILES)
