# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

include $(top_srcdir)/config/Make.rules.common

ifeq ($(MinGW), yes)
	DLLNAME		= $(top_srcdir)/bin/iceutil$(COMPSUFFIX)$(SOVERSION)$(LIBSUFFIX).dll
	TARGETS		= $(DLLNAME) $(LIBNAME)
else
	LIBFILENAME	= $(call mklibfilename,IceUtil,$(VERSION))
	SONAME		= $(call mksoname,IceUtil,$(SOVERSION))  
	LIBNAME		= $(call mklibname,IceUtil)
	TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
endif

OBJS		= ArgVector.o \
		  Cond.o \
		  ConvertUTF.o \
		  CountDownLatch.o \
		  CtrlCHandler.o \
		  Exception.o \
		  InputUtil.o \
		  Options.o \
		  OutputUtil.o \
		  Random.o \
		  RecMutex.o \
		  Shared.o \
		  StringUtil.o \
		  Thread.o \
		  ThreadException.o \
		  Time.o \
		  Timer.o \
		  UUID.o \
		  Unicode.o \
		  MutexProtocol.o \
		  FileUtil.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS        := $(CPPFLAGS) $(ICEUTIL_FLAGS) -DICE_UTIL_API_EXPORTS -I..

LINKWITH        := $(ICEUTIL_OS_LIBS)

ifeq ($(MinGW), yes)
$(DLLNAME): $(OBJS)
	$(CXX) -shared $(LDFLAGS) -o $(DLLNAME) $(OBJS) $(LINKWITH)
	
install:: all
	$(call installprogram,$(DLLNAME),$(install_bindir))
else
ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)/$(LIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(SONAME) $@
endif
install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
endif

include .depend

parser: parser.o
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ parser.o $(BASELIBS)

