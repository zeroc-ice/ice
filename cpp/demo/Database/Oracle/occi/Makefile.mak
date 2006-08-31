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

SLICE_SRCS	= HR.ice

OBJS		= HR.obj

COBJS		= Client.obj

SOBJS		= DbTypes.obj \
                  DbTypesMap.obj \
                  DeptFactoryI.obj \
                  DeptI.obj  \
                  EmpI.obj  \
                  Server.obj \
                  Util.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I$(ORACLE_HOME)\precomp\public $(CPPFLAGS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

ORACLE_LIBS     = /LIBPATH:$(ORACLE_HOME)\precomp\lib oraocci10$(LIBSUFFIX).lib

$(CLIENT): $(OBJS) $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	rm -f $@
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(ORACLE_LIBS)


DbTypes.h DbTypes.cpp DbTypesMap.h DbTypesMap.cpp DbTypesOut.typ: DbTypes.typ
	ott userid=scott/tiger@orcl code=cpp hfile=DbTypes.h cppfile=DbTypes.cpp mapfile=DbTypesMap.cpp \
	  intype=DbTypes outtype=DbTypesOut.typ attraccess=private

clean::
	del /q HR.cpp HR.h
	del /q DbTypes.h DbTypes.cpp DbTypesMap.h DbTypesMap.cpp DbTypesOut.typ

!include .depend
