# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBFILENAME	= $(call mklibfilename,Slice,$(VERSION))
SONAME		= $(call mksoname,Slice,$(SOVERSION))
LIBNAME		= $(call mklibname,Slice)
TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

OBJS		= Checksum.o \
		  CPlusPlusUtil.o \
		  CsUtil.o \
		  DotNetNames.o \
		  FileTracker.o \
		  JavaUtil.o \
		  MD5.o \
		  MD5I.o \
		  ObjCUtil.o \
		  Parser.o \
		  PHPUtil.o \
		  Preprocessor.o \
		  PythonUtil.o \
		  Python.o \
		  RubyUtil.o \
		  Ruby.o \
		  Scanner.o \
		  Util.o \
		  ../Slice/Grammar.o

include $(top_srcdir)/config/Make.rules


CPPFLAGS	:= -I.. $(CPPFLAGS) -DSLICE_API_EXPORTS
BISONFLAGS	:= --name-prefix "slice_" $(BISONFLAGS)

LINKWITH	:= $(BASELIBS) $(MCPP_LIBS)
ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS) $(MCPP_NUPKG)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mkshlib,$@,$(SONAME),$(OBJS),$(LINKWITH))

$(libdir)/$(SONAME): $(libdir)/$(LIBFILENAME)
	rm -f $@
	ln -s $(LIBFILENAME) $@

$(libdir)$(cpp11libdirsuffix)/$(LIBNAME): $(libdir)/$(SONAME)
	@mkdir -p $(libdir)$(cpp11libdirsuffix)
	rm -f $@
	ln -s $(cpp11sonamedir)$(SONAME) $@

install:: all
	$(call installlib,$(DESTDIR)$(install_libdir),$(libdir),$(LIBFILENAME),$(SONAME),$(LIBNAME))
endif

