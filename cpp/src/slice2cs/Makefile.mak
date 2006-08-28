# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

NAME		= $(top_srcdir)\bin\slice2cs.exe

TARGETS		= $(NAME)

OBJS		= Gen.obj \
		  Main.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS)

$(NAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS), $@,, slice$(LIBSUFFIX).lib $(BASELIBS)

install:: all
	copy $(NAME) $(install_bindir)

!include .depend
