# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

TOOL		= $(top_srcdir)\bin\iceserviceinstall.exe

TARGETS         = $(TOOL)

OBJS            = ServiceInstaller.obj \
                  Install.obj

SRCS		= $(OBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

LINKWITH        = $(LIBS)
!if "$(CPP_COMPILER)" == "VC90" || "$(CPP_COMPILER)" == "VC90_EXPRESS"
LINKWITH	= /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'" $(LINKWITH)
!else
EXTRA_MANIFEST  = security.manifest
!endif

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS       = /pdb:$(TOOL:.exe=.pdb)
!endif

$(TOOL): $(OBJS) IceServiceInstall.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) IceServiceInstall.res $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest $(EXTRA_MANIFEST) -outputresource:$@;#1 && del /q $@.manifest

IceServiceInstall.res: IceServiceInstall.rc
	rc.exe $(RCFLAGS) IceServiceInstall.rc


clean::
	del /q $(TOOL:.exe=.*)
	del /q IceServiceInstall.res

install:: all
	copy $(TOOL) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(TOOL:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(TOOL:.exe=.pdb) $(install_bindir)

!endif

!endif


!include .depend

