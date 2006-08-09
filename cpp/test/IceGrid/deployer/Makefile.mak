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

OBJS		= Test.o

COBJS		= Client.o \
		  AllTests.o

SOBJS		= TestI.o \
		  Server.o

SERVICE_OBJS	= TestI.o \
		  Service.o

SRCS		= $(OBJS:.o=.cpp) \
		  $(COBJS:.o=.cpp) \
		  $(SOBJS:.o=.cpp) \
		  $(SERVICE_OBJS:.o=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. -I../../include $(CPPFLAGS)
LINKWITH	= $(LIBS) icebox$(LIBSUFFIX).lib

$(LIBNAME) : $(DLLNAME)

$(DLLNAME): $(OBJS) $(SERVICE_OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(OBJS) $(SERVICE_OBJS), $(DLLNAME),, $(LINKWITH) freeze$(LIBSUFFIX).lib

$(CLIENT): $(OBJS) $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(COBJS), $@,, $(LINKWITH) icegrid$(LIBSUFFIX).lib glacier2$(LIBSUFFIX).lib

$(SERVER): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(SOBJS), $@,, $(LIBS)

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
