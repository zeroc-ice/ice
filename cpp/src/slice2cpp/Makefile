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
	NAME		= $(top_srcdir)/bin/slice2cpp.exe
else
	NAME		= $(top_srcdir)/bin/slice2cpp
endif

TARGETS		= $(NAME)

OBJS		= Gen.o \
		  Main.o

SRCS		= $(OBJS:.o=.cpp)

include $(top_srcdir)/config/Make.rules

CPPFLAGS	:= -I. $(CPPFLAGS)

ifeq ($(MinGW), yes)
$(NAME): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lslice$(COMPSUFFIX)$(SOVERSION)$(LIBSUFFIX) $(BASELIBS) $(MCPP_RPATH_LINK)
else
$(NAME): $(OBJS)
	rm -f $@
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) -lSlice $(BASELIBS) $(MCPP_RPATH_LINK)
endif
install:: all
	$(call installprogram,$(NAME),$(install_bindir))

include .depend
