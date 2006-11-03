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

OBJS		= Counter.obj

COBJS		= Client.obj \
		  CounterObserverI.obj

SOBJS		= Server.obj \
		  CounterI.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)


!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN
IS_LIBS		= $(top_srcdir)\lib\icestorm$(LIBSUFFIX).lib $(LIBS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PPDBFLAGS        = /pdb:$(CLIENT:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(CLIENT): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(PPDBFLAGS) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(IS_LIBS)

clean::
	del /q Counter.cpp Counter.h

!include .depend
