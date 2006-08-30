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

OBJS		= cccp.obj \
		  cexp.obj \
		  prefix.obj

!include $(top_srcdir)/config/Make.rules.mak

CFLAGS = $(CFLAGS) -I. -I../../include

!if "$(BORLAND_HOME)" == ""
LINKWITH	= setargv.obj advapi32.lib
!endif

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(NAME:.exe=.pdb)
!endif

cexp.c: cexp.y
	bison -o cexp.c cexp.y

prefix.obj: prefix.c
	$(CC) /c $(CPPFLAGS) $(CFLAGS) -D__STDC__ prefix.c

$(NAME): $(OBJS)
	del /q $@
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)

install:: all
	copy $(NAME) $(install_bindir)

clean::
	del /q cexp.c
	del /q $(NAME:.exe=.*)
