# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

CLIENT		= makedb.exe

TARGETS		= $(CLIENT)

OBJS		= TestOld.o \
                  makedb.o

SRCS		= $(OBJS:.o=.cpp)

SLICE_SRCS	= TestOld.ice

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS)

$(CLIENT): $(OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS), $@,, $(LIBS) freeze$(LIBSUFFIX).lib

TestOld.cpp TestOld.h: TestOld.ice $(SLICE2CPP) $(SLICEPARSERLIB)
        $(SLICE2CPP) $(SLICE2CPPFLAGS) TestOld.ice

clean::
	del /q TestOld.cpp TestOld.h

clean::
	del /q db\*.db db\log.* db\__catalog

!include .depend
