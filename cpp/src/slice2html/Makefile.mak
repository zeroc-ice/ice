# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

NAME		= $(top_srcdir)\bin\slice2html.exe

TARGETS		= $(NAME)

OBJS		= Gen.obj \
		  Main.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(BORLAND_HOME)" == "" & "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(NAME:.exe=.pdb)
!endif

$(NAME): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)slice$(LIBSUFFIX).lib $(BASELIBS)

clean::
	del /q $(NAME:.exe=.*)

install:: all
	copy $(NAME) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(BORLAND_HOME)" == ""

install:: all
	copy $(NAME:.exe=.pdb) $(install_bindir)

!else

install:: all
	copy $(NAME:.exe=.tds) $(install_bindir)

!endif

!endif

!include .depend
