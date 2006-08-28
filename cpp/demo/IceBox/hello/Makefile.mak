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

LIBNAME     	= helloservice$(LIBSUFFIX).lib
DLLNAME         = helloservice$(LIBSUFFIX).dll

TARGETS		= $(CLIENT) $(LIBNAME) $(DLLNAME)

OBJS		= Hello.obj

COBJS		= Client.obj

SOBJS		= HelloI.obj \
		  HelloServiceI.obj

SRCS		= $(OBJS:.obj=.cpp) \
		  $(COBJS:.obj=.cpp) \
		  $(SOBJS:.obj=.cpp)

SLICE_SRCS	= Hello.ice

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS)
LINKWITH	= $(LIBS) icebox$(LIBSUFFIX).lib

$(LIBNAME) : $(DLLNAME)

$(DLLNAME): $(OBJS) $(SOBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(OBJS) $(SOBJS), $(DLLNAME),, $(LINKWITH)

$(CLIENT): $(OBJS) $(COBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS) $(COBJS), $@,, $(LIBS)

Hello.cpp Hello.h: Hello.ice $(SLICE2CPP) $(SLICEPARSERLIB)
	$(SLICE2CPP) $(SLICE2CPPFLAGS) Hello.ice

clean::
	del /q Hello.cpp Hello.h

!include .depend
