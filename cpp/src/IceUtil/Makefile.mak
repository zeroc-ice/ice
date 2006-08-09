# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\iceutil$(LIBSUFFIX).lib
DLLNAME         = $(top_srcdir)\bin\iceutil$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= Base64.o \
		  Cond.o \
                  ConvertUTF.o \
		  CountDownLatch.o \
		  CtrlCHandler.o \
		  Exception.o \
		  Shared.o \
		  InputUtil.o \
		  MD5.o \
		  MD5I.o \
		  Options.o \
		  OutputUtil.o \
		  Random.o \
		  RWRecMutex.o \
		  RecMutex.o \
		  StaticMutex.o \
		  StringUtil.o \
		  Thread.o \
		  ThreadException.o \
		  Time.o \
		  UUID.o \
		  Unicode.o

SRCS		= $(OBJS:.o=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS        = $(CPPFLAGS) -DICE_UTIL_API_EXPORTS -I..
LINKWITH        = $(STLPORT_LIBS) $(ICEUTIL_OS_LIBS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_DLLFLAGS) $(OBJS), $(DLLNAME),, $(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!include .depend

parser: parser.o
	del /q $@
	$(CXX) $(LDFLAGS) -o $@ parser.o $(BASELIBS)

