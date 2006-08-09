# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

NAME		= $(top_srcdir)\bin\icecpp.exe

TARGETS		= $(NAME)

OBJS		= cccp.o \
		  cexp.o \
		  prefix.o

!include $(top_srcdir)/config/Make.rules.mak

CFLAGS = $(CFLAGS) -I. -DPREFIX=\"\" -DUSE_PROTOTYPES

cexp.c: cexp.y
	bison -o cexp.c cexp.y

$(NAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(OBJS), $@,,

install:: all
	copy $(NAME) $(install_bindir)

clean::
	del /q cexp.c
