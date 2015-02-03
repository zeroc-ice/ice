# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

!if exist ($(top_srcdir)\..\.gitignore)
MAXWARN         = yes
!endif

CLIENT		= client.exe
SERVER		= server.exe

TARGETS		= $(CLIENT) $(SERVER)

PROC_SRCS 	= .\CurrentSqlContext.pc \
		  .\DeptFactoryI.pc \
		  .\DeptI.pc \
		  .\EmpI.pc \
		  .\Server.pc \
		  .\Util.pc

SLICE_OBJS	= .\HR.obj

COBJS		= $(SLICE_OBJS) \
		  .\Client.obj

SOBJS		= $(SLICE_OBJS) \
		$(PROC_SRCS:.pc=.obj)

OBJS		= $(COBJS) \
		  $(SOBJS)

!include $(top_srcdir)/config/Make.rules.mak

#
# Oracle
#
!if "$(ORACLE_CLIENT_HOME)" == ""
ORACLE_LIBDIR 		= $(ORACLE_HOME)\precomp\LIB
ORACLE_INCLUDEDIR	= $(ORACLE_HOME)\precomp\public
!else
ORACLE_LIBDIR 		= $(ORACLE_CLIENT_HOME)\precomp\LIB
ORACLE_INCLUDEDIR	= $(ORACLE_CLIENT_HOME)\precomp\public
!endif

#
# Change to orasql11.lib if you're linking with Oracle 11
#
ORACLE_LIBS     = -LIBPATH:"$(ORACLE_LIBDIR)" orasql12.lib

CPPFLAGS	= -I. -I"$(ORACLE_INCLUDEDIR)" -DSQLCA_NONE $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

GENERATED_PROC_FILES  = $(PROC_SRCS:.pc=.cpp)

.SUFFIXES:
.SUFFIXES:		.ice .pc .cpp .c .obj

.pc.cpp:
	proc threads=yes parse=none lines=yes code=cpp cpp_suffix=cpp close_on_commit=yes $<

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
		$(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(ORACLE_LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
		$(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q HR.cpp HR.h
	del /q $(GENERATED_PROC_FILES)
