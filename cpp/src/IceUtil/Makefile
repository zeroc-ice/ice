# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,IceUtil,$(VERSION))
SONAME          = $(call mksoname,IceUtil,$(SOVERSION))  
LIBNAME		= $(call mklibname,IceUtil)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= Base64.o \
		  Cond.o \
		  CountDownLatch.o \
		  CtrlCHandler.o \
		  Exception.o \
		  GC.o \
		  GCRecMutex.o \
		  GCShared.o \
		  InputUtil.o \
		  MD5.o \
		  MD5I.o \
		  Options.o \
		  OutputUtil.o \
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

include $(top_srcdir)/config/Make.rules
ifeq ($(USE_SPARC_ASM),yes)
ASRC            =       IceAtomic.c
AOBJ            =       IceAtomic.o
SRCS            :=      $(SRCS) $(ASRC)
OBJS            :=      $(OBJS) $(AOBJ)
CFLAGS          :=      $(CFLAGS) -fomit-frame-pointer
endif


CPPFLAGS        := $(CPPFLAGS) -DICE_UTIL_API_EXPORTS -I..

LINKWITH        := $(STLPORT_LIBS) $(ICEUTIL_OS_LIBS)

$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))

include .depend

parser: parser.o
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ parser.o $(BASELIBS)

