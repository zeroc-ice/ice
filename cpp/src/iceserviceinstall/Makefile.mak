# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

TOOL		= $(top_srcdir)\bin\iceserviceinstall.exe

TARGETS         = $(TOOL)

OBJS		= Install.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS       = /pdb:$(TOOL:.exe=.pdb)
!endif


$(TOOL): $(OBJS)
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LIBS)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#1 && del /q $@.manifest

clean::
	del /q $(TOOL:.exe=.*)

install:: all
	copy $(TOOL) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(TOOL:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(TOOOL:.exe=.pdb) $(install_bindir)

!endif

!endif

!include .depend
