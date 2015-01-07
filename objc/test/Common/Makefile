# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Touch is licensed to you under the terms described in the
# ICE_TOUCH_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../..

LIBNAME		= $(call mklibname,TestCommon)

TARGETS		= ../../lib/$(LIBNAME)

OBJS		= TestCommon.o

SRCS		:= $(OBJS:.o=.m)
DEPENDFLAGS	= --obj-dir

include $(top_srcdir)/config/Make.rules

ifeq ($(STATICLIBS),yes)
    mklibname	= lib$(1).a
else
    mklibname	= lib$(1).dylib
endif

OBJS		:= $(addprefix $(OBJDIR)/,$(OBJS))

CPPFLAGS	:= -I../include $(CPPFLAGS)

ifeq ($(STATICLIBS),yes)

../../lib/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mklib,$@,$(OBJS))

else

../../lib/$(LIBNAME): $(OBJS)
	rm -f $@
	$(call mkshlib,$@,$(LIBNAME),$(OBJS),$(LIBS))

endif

$(OBJDIR)/%.o: %.m
	$(CC) -c $(CPPFLAGS) $(OBJCFLAGS) $(CFLAGS) $< -o $@


