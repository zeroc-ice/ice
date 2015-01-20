# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..\..

CLIENT		= client.exe
SERVER		= server.exe

TARGETS		= $(CLIENT) $(SERVER)

SLICE_OBJS	= .\HR.obj

COBJS		= $(SLICE_OBJS) \
		  .\Client.obj

SOBJS		= $(SLICE_OBJS) \
		  .\DbTypes.obj \
		  .\DbTypesMap.obj \
		  .\DeptFactoryI.obj \
		  .\DeptI.obj  \
		  .\EmpI.obj  \
		  .\OCCIServantLocator.obj \
		  .\Server.obj \
		  .\Util.obj

OBJS		= $(COBJS) \
		  $(SOBJS)

!include $(top_srcdir)\config\Make.rules.mak


!if "$(GENERATE_PDB)" == "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

#
# OCCI
#
!if "$(CPP_COMPILER)" == "VC120"
OCCI_LIBSUBDIR = VC12
!else if "$(CPP_COMPILER)" == "VC110"
OCCI_LIBSUBDIR = VC11
!else
!error "$(CPP_COMPILER) is not supported by this demo"
!endif

#
# Oracle instantclient home
#
!if "$(ORACLE_INSTANTCLIENT_HOME)" == ""
OCCI_LIBDIR 	= $(ORACLE_HOME)\oci\lib\msvc
OCCI_INCLUDEDIR	= $(ORACLE_HOME)\oci\include
!else
OCCI_LIBDIR 	= $(ORACLE_INSTANTCLIENT_HOME)\sdk\lib\msvc
OCCI_INCLUDEDIR	= $(ORACLE_INSTANTCLIENT_HOME)\sdk\include
!endif

#
# OCCI libraries default location, adjust to match your setup.
#
ORACLE_LIBS     = -LIBPATH:"$(OCCI_LIBDIR)\msvc\$(OCCI_LIBSUBDIR)" oraocci12$(LIBSUFFIX).lib -LIBPATH:"$(OCCI_LIBDIR)" oci.lib

#
# Disable warnings 4101 and 4291 issued when compiling DbTypes.cpp
#
CPPFLAGS	= -I. -I$(OCCI_INCLUDEDIR) -wd4101 -wd4291 $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

$(CLIENT): $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
		$(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

$(SERVER): $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(ORACLE_LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
		$(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

DbTypes.h DbTypes.cpp DbTypesMap.h DbTypesMap.cpp DbTypesOut.typ: DbTypes.typ
	ott userid=scott/tiger@orcl code=cpp hfile=DbTypes.h cppfile=DbTypes.cpp mapfile=DbTypesMap.cpp \
	  intype=DbTypes.typ outtype=DbTypesOut.typ attraccess=private

clean::
	del /q HR.cpp HR.h
	del /q DbTypes.h DbTypes.cpp DbTypesMap.h DbTypesMap.cpp DbTypesOut.typ
