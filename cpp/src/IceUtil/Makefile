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

TARGETS		= $(libdir)/$(LIBFILENAME) $(libdir)/$(SONAME) $(libdir)/$(LIBNAME)

OBJS		= Exception.o \
		  Unicode.o \
		  UUID.o \
		  RecMutex.o \
		  RWRecMutex.o \
		  Cond.o \
		  Thread.o \
		  ThreadException.o \
                  StaticMutex.o \
                  CtrlCHandler.o \
		  Time.o \
		  StringUtil.o \
		  InputUtil.o \
		  OutputUtil.o \
                  Base64.o \
		  GCShared.o \
		  GCRecMutex.o \
		  GC.o \
                  CountDownLatch.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules
ifeq ($(USE_SPARC_ASM),yes)
ASRC            =       IceAtomic.c
AOBJ            =       IceAtomic.o
SRCS            :=      $(SRCS) $(ASRC)
OBJS            :=      $(OBJS) $(AOBJ)
CFLAGS          :=      $(CFLAGS) -fomit-frame-pointer
endif


CPPFLAGS        := $(CPPFLAGS) -DICE_UTIL_API_EXPORTS

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
	$(INSTALL_LIBRARY) $(libdir)/$(LIBFILENAME) $(install_libdir)
	rm -f $(install_libdir)/$(SONAME)
	ln -s $(LIBFILENAME) $(install_libdir)/$(SONAME)
	rm -f $(install_libdir)/$(LIBNAME)
	ln -s $(SONAME) $(install_libdir)/$(LIBNAME)

include .depend

parser: parser.o
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ parser.o $(BASELIBS)

