# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

CPPFLAGS	= -I. -I"$(ORACLE_HOME)\precomp\public" -DSQLCA_NONE $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

GENERATED_PROC_FILES  = $(PROC_SRCS:.pc=.cpp) 

#
# Change to orasql10.lib if you're linking with Oracle 10
#
ORACLE_LIBS     = -LIBPATH:"$(ORACLE_HOME)\precomp\lib" orasql11.lib

.SUFFIXES:
.SUFFIXES:		.ice .pc .cpp .c .obj

.pc.cpp:
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $<

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(ORACLE_LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q HR.cpp HR.h
	del /q $(GENERATED_PROC_FILES)

!include .depend.mak
