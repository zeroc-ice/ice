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

TARGETS		= $(CLIENT) $(SERVER)

OBJS		= Hello.obj

COBJS		= Client.obj

SOBJS		= HelloI.obj \
		  Server.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
CPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
	  icegrid$(LIBSUFFIX).lib glacier2$(LIBSUFFIX).lib

$(SERVER): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

Hello.cpp Hello.h: Hello.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Hello.ice

clean::
	del /q Hello.cpp Hello.h

!include .depend
