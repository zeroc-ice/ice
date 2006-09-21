# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

CLIENT		= client.exe
SERVER		= server.exe

TARGETS		= $(CLIENT) $(SERVER)

PROC_SRCS       = CurrentSqlContext.pc \
                  Util.pc \
                  EmpI.pc \
                  DeptI.pc \
                  DeptFactoryI.pc \
                  Server.pc

OBJS		= HR.obj

COBJS		= Client.obj

SOBJS		= $(PROC_SRCS:.pc=.obj)

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I$(ORACLE_HOME)\precomp\public -DSQLCA_NONE $(CPPFLAGS)

GENERATED_PROC_FILES  = $(PROC_SRCS:.pc=.cpp) 

ORACLE_LIBS     = -L$(ORACLE_HOME)\precomp\lib orasql10$(LIBSUFFIX).lib

.SUFFIXES:
.SUFFIXES:		.pc .cpp .c .o

#
# The rm -f tp* $*.lis is to work around a proc bug on Linux
#
.pc.cpp:
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $<
	del /q tp* $*.lis

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(ORACLE_LIBS)

clean::
	del /q HR.cpp HR.h
	del /q $(GENERATED_PROC_FILES)

!include .depend
