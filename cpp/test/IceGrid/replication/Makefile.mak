# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= client.exe
SERVER		= server.exe

LIBNAME		= testservice$(LIBSUFFIX).lib
DLLNAME		= testservice$(LIBSUFFIX).dll

TARGETS		= $(CLIENT) $(SERVER) $(LIBNAME) $(DLLNAME)

OBJS		= Test.obj

COBJS		= Client.obj \
		  AllTests.obj

SOBJS		= TestI.obj \
		  Server.obj

SERVICE_OBJS	= TestI.obj \
		  Service.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(SERVICE_OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS)
LINKWITH	= $(LIBS) icebox$(LIBSUFFIX).lib

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
CPDBFLAGS       = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS       = /pdb:$(SERVER:.exe=.pdb)
!endif

$(LIBNAME) : $(DLLNAME)

$(DLLNAME): $(OBJS) $(SERVICE_OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(SERVICE_OBJS) $(PREOUT)$(DLLNAME) $(PRELIBS)$(LINKWITH) \
	  freeze$(LIBSUFFIX).lib

$(CLIENT): $(OBJS) $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) \
	  icegrid$(LIBSUFFIX).lib glacier2$(LIBSUFFIX).lib

$(SERVER): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

Test.cpp Test.h: Test.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Test.ice

clean::
	del /q Test.cpp Test.h

!if "$(OPTIMIZE)" == "yes"

all::
	@echo release > build.txt

!else

all::
	@echo debug > build.txt

!endif

!include .depend
