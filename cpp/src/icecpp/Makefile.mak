# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

CFLAGS = $(CFLAGS) -I. -I../../include -DWIN32_LEAN_AND_MEAN

!if "$(CPP_COMPILER)" != "BCC2006"
LINKWITH	= advapi32.lib
!endif

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(NAME:.exe=.pdb)
!endif

cexp.c: cexp.y
	bison -o cexp.c cexp.y

prefix.obj: prefix.c
	$(CC) /c $(CPPFLAGS) $(CFLAGS) -D__STDC__ prefix.c

$(NAME): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

install:: all
	copy $(NAME) $(install_bindir)

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"

install:: all
	copy $(NAME:.exe=.pdb) $(install_bindir)

!endif

clean::
	del /q cexp.c
	del /q $(NAME:.exe=.*)
