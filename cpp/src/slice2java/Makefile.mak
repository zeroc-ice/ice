# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

NAME		= $(top_srcdir)\bin\slice2java.exe

TARGETS		= $(NAME)

OBJS		= Gen.obj \
		  Main.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(NAME:.exe=.pdb)
!endif

$(NAME): $(OBJS) Slice2Java.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) Slice2Java.res $(SETARGV) $(PREOUT)$@ \
		$(PRELIBS)slice$(LIBSUFFIX).lib $(BASELIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest

Slice2Java.res: Slice2Java.rc
	rc.exe $(RCFLAGS) Slice2Java.rc

clean::
	del /q $(NAME:.exe=.*)
	del /q Slice2Java.res

install:: all
	copy $(NAME) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(NAME:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(NAME:.exe=.pdb) $(install_bindir)

!endif

!endif

!include .depend
