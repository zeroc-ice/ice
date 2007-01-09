# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

PUBLISHER	= publisher.exe
CONSUMER	= consumer.exe
SERVER		= server.exe

TARGETS		= $(PUBLISHER) $(CONSUMER) $(SERVER)

OBJS		= Queue.obj

POBJS		= Publisher.obj

COBJS		= Consumer.obj

SOBJS		= QueueI.obj \
		  Server.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp) \
		  $(POBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(CPP_COMPILER)" != "BCC2006" & "$(OPTIMIZE)" != "yes"
PPDBFLAGS        = /pdb:$(PUBLISHER:.exe=.pdb)
CPDBFLAGS        = /pdb:$(CONSUMER:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SERVER:.exe=.pdb)
!endif

$(PUBLISHER): $(OBJS) $(POBJS)
	$(LINK) $(LD_EXEFLAGS) $(PPDBFLAGS) $(OBJS) $(POBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(CONSUMER): $(OBJS) $(COBJS)
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(OBJS) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(SERVER): $(OBJS) $(SOBJS)
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

clean::
	del /q Queue.cpp Queue.h

!include .depend
