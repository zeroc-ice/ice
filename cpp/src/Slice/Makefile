# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME     = $(call mklibfilename,Slice,$(VERSION))
SONAME          = $(call mksoname,Slice,$(SOVERSION))  
LIBNAME		= $(call mklibname,Slice)

TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)/$(LIBNAME))

OBJS		= Scanner.o \
		  Grammar.o \
		  Parser.o \
		  CPlusPlusUtil.o \
		  CsUtil.o \
		  VbUtil.o \
		  JavaUtil.o \
		  Preprocessor.o \
		  Checksum.o \
		  PythonUtil.o \
		  DotNetNames.o \
		  RubyUtil.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I.. $(CPPFLAGS) -DSLICE_API_EXPORTS 
LINKWITH	:= -lIceUtil
BISONFLAGS	:= --name-prefix "slice_" $(BISONFLAGS)

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

#
# The .y rule does not work well with parallel builds; need to investigate
#
.NOTPARALLEL:

clean::
	rm -f Grammar.cpp Grammar.h
	rm -f Scanner.cpp

install:: all
	$(call installlib,$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))


include .depend
