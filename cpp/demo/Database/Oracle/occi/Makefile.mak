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

SLICE_SRCS	= HR.ice

OBJS		= HR.obj

COBJS		= Client.obj

SOBJS		= DbTypes.obj \
                  DbTypesMap.obj \
                  DeptFactoryI.obj \
                  DeptI.obj  \
                  EmpI.obj  \
                  OCCIServantLocator.obj \
                  Server.obj \
                  Util.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

#
# Disable warnings 4101 and 4291 issued when compiling DbTypes.cpp
#
CPPFLAGS	= -I. -I$(ORACLE_HOME)\oci\include -wd4101 -wd4291 $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

#
# OCCI first
#
!if "$(CPP_COMPILER)" == "VC100"

#
# OCCI libraries default location, adjust to match your setup.
#
ORACLE_LIBS     = -LIBPATH:"$(ORACLE_HOME)\oci\lib\msvc\vc10" oraocci11$(LIBSUFFIX).lib
!else
!error "$(CPP_COMPILER) is not supported by this demo"
!endif

#
# OCI
#
ORACLE_LIBS = $(ORACLE_LIBS) -LIBPATH:"$(ORACLE_HOME)\oci\lib\msvc" oci.lib


$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(ORACLE_LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

DbTypes.h DbTypes.cpp DbTypesMap.h DbTypesMap.cpp DbTypesOut.typ: DbTypes.typ
	ott userid=scott/tiger@orcl code=cpp hfile=DbTypes.h cppfile=DbTypes.cpp mapfile=DbTypesMap.cpp \
	  intype=DbTypes.typ outtype=DbTypesOut.typ attraccess=private

clean::
	del /q HR.cpp HR.h
	del /q DbTypes.h DbTypes.cpp DbTypesMap.h DbTypesMap.cpp DbTypesOut.typ

!include .depend.mak
