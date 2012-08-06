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
	DLLNAME		= $(top_srcdir)/bin/slice$(COMPSUFFIX)$(SOVERSION)$(LIBSUFFIX).dll
	TARGETS		= $(DLLNAME) $(LIBNAME)
else
	LIBFILENAME	= $(call mklibfilename,Slice,$(VERSION))
	SONAME		= $(call mksoname,Slice,$(SOVERSION))  
	LIBNAME		= $(call mklibname,Slice)
	TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))
endif

OBJS		= Scanner.o \
		  ../Slice/Grammar.o \
		  Parser.o \
		  CPlusPlusUtil.o \
		  CsUtil.o \
		  JavaUtil.o \
		  Preprocessor.o \
		  Checksum.o \
		  PythonUtil.o \
		  DotNetNames.o \
		  RubyUtil.o \
		  PHPUtil.o \
		  Util.o \
		  FileTracker.o \
		  MD5.o \
		  MD5I.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules


CPPFLAGS	:= -I.. $(CPPFLAGS) -DSLICE_API_EXPORTS 
BISONFLAGS	:= --name-prefix "slice_" $(BISONFLAGS)

ifeq ($(MinGW), yes)
LINKWITH	:= -l$(ICEUTIL_LIBNAME) $(MCPP_LIBS)
$(DLLNAME): $(OBJS)
	$(CXX) -shared $(LDFLAGS) -o $(DLLNAME) $(OBJS) $(LINKWITH)
	
install:: all
	$(call installprogram,$(DLLNAME),$(install_bindir))
else
LINKWITH	:= -lIceUtil $(MCPP_LIBS)
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
