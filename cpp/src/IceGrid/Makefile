# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

PERFLIBFILENAME = $(call mklibfilename,IcePerf,$(VERSION))
PERFSONAME      = $(call mksoname,IcePerf,$(SOVERSION))  
PERFLIBNAME	= $(call mklibname,IcePerf)


LIBTARGETS	= $(call mklibtargets,$(libdir)/$(PERFLIBFILENAME),$(libdir)/$(PERFSONAME),$(libdir)/$(PERFLIBNAME))
TARGETS         = $(LIBTARGETS)

PERFLIB_OBJS	= PerfTypes.o \
                  PerfLinux.o

SRCS		= $(PERFLIB_OBJS:.o=.cpp)

SLICE_SRCS	= $(SDIR)/PerfTypes.ice

HDIR		= $(includedir)/IceGrid
LOCAL_HDIR	= ../IceGrid
SDIR		= $(slicedir)/IceGrid
LOCAL_SDIR	= ../IceGrid

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= $(CPPFLAGS) -DICE_PERF_API_EXPORTS
SLICE2CPPFLAGS 	:= --ice --include-dir IceGrid --dll-export ICE_PERF_API $(SLICE2CPPFLAGS)
LINKWITH        := -lIce -lIceUtil

$(libdir)/$(PERFLIBFILENAME): $(PERFLIB_OBJS)
	rm -f $@
	$(call mkshlib,$@,$(PERFSONAME),$(PERFLIB_OBJS),$(LINKWITH))

$(libdir)/$(PERFSONAME): $(libdir)/$(PERFLIBFILENAME)
	rm -f $@
	ln -s $(PERFLIBFILENAME) $@

$(libdir)/$(PERFLIBNAME): $(libdir)/$(SONAME)
	rm -f $@
	ln -s $(PERFSONAME) $@

$(LOCAL_HDIR)/%.h %.cpp: $(SDIR)/%.ice $(SLICE2CPP)
	rm -f $(HDIR)/$(*F).h $(*F).cpp
	$(SLICE2CPP) $(SLICE2CPPFLAGS) $(SDIR)/$(*F).ice


install:: all
	$(call installlib,$(install_libdir),$(libdir),$(PERFLIBFILENAME),$(PERFSONAME),$(PERFLIBNAME))

include .depend
