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

OBJS		= Event.obj

POBJS		= Publisher.obj

SOBJS		= Subscriber.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(POBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)


!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS)
LIBS		= icestorm$(LIBSUFFIX).lib $(LIBS)

$(PUBLISHER): $(OBJS) $(POBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(POBJS), $@,, $(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(SOBJS), $@,, $(LIBS)

Event.cpp Event.h: Event.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Event.ice

clean::
	del /q Event.cpp Event.h

!if "$(OPTIMIZE)" == "yes"

all::
	@echo release > build.txt

!else

all::
	@echo debug > build.txt
!endif

clean::
	del /q db\topicmanager db\log.*

!include .depend
