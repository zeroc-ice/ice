# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

PUBLISHER	= publisher.exe
SUBSCRIBER	= subscriber.exe

TARGETS		= $(PUBLISHER) $(SUBSCRIBER)

OBJS		= Clock.obj

POBJS		= Publisher.obj

SOBJS		= Subscriber.obj \
		  ClockI.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(POBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)


!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS)
LIBS		= $(top_srcdir)\lib\icestorm$(LIBSUFFIX).lib $(LIBS)

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PPDBFLAGS        = /pdb:$(PUBLISHER:.exe=.pdb)
SPDBFLAGS        = /pdb:$(SUBSCRIBER:.exe=.pdb)
!endif

$(PUBLISHER): $(OBJS) $(POBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(PPDBFLAGS) $(OBJS) $(POBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(OBJS) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS)

clean::
	del /q Clock.cpp Clock.h

!include .depend
