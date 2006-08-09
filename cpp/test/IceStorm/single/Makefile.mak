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

OBJS		= Single.o

POBJS		= Publisher.o

SOBJS		= Subscriber.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(POBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp)


!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS)
LIBS		= icestorm$(LIBSUFFIX).lib $(LIBS)

$(PUBLISHER): $(OBJS) $(POBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(POBJS), $@,, $(LIBS)

$(SUBSCRIBER): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(SOBJS), $@,, $(LIBS)

Single.cpp Single.h: Single.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Single.ice

clean::
	del /q Single.cpp Single.h

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
