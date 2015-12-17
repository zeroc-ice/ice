# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

testincdir	= ../include

LIBFILENAME	= $(call mklibfilename,TestCommon,$(VERSION))
SONAME		= $(call mksoname,TestCommon,$(SOVERSION))
LIBNAME		= $(call mklibname,TestCommon)
TARGETS		= $(call mklibtargets,$(libdir)/$(LIBFILENAME),$(libdir)/$(SONAME),$(libdir)$(cpp11libdirsuffix)/$(LIBNAME))

SLICE_OBJS	= Controller.o

OBJS		= TestCommon.o \
		  $(SLICE_OBJS)

HDIR		= $(testincdir)
SDIR		= .

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I$(testincdir) $(CPPFLAGS) -DTEST_API_EXPORTS
SLICE2CPPFLAGS	:= --dll-export TEST_API $(SLICE2CPPFLAGS)

LINKWITH        := $(LIBS)

ifeq ($(STATICLIBS),yes)
$(libdir)/$(LIBNAME): $(OBJS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(call mklib,$@,$(OBJS))
else
$(libdir)/$(LIBFILENAME): $(OBJS)
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
endif
