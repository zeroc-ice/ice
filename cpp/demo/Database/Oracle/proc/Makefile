# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../../..

CLIENT		= client
SERVER		= server

TARGETS		= $(CLIENT) $(SERVER)

SLICE_SRCS	= HR.ice

PROC_SRCS       = CurrentSqlContext.pc \
                  Util.pc \
                  EmpI.pc \
                  DeptI.pc \
                  DeptFactoryI.pc \
                  Server.pc

OBJS		= HR.o

COBJS		= Client.o

SOBJS		= $(PROC_SRCS:.pc=.o)

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)



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

$(CLIENT): $(OBJS) $(COBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SOBJS) $(ORACLE_LIBS) $(LIBS)

clean::
	rm -f $(GENERATED_PROC_FILES)

include .depend
