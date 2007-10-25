# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#!define HAS_MSI_SDK

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\iceserviceinstaller$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\iceserviceinstaller$(SOVERSION)$(LIBSUFFIX).dll

TOOL		= $(top_srcdir)\bin\iceserviceinstall.exe

!ifdef BUILD_UTILS

TARGETS         = $(TOOL)

!else

TARGETS         = $(LIBNAME) $(DLLNAME)

!endif

TOBJS		= Install.obj

OBJS           = ServiceInstaller.obj \
!ifdef HAS_MSI_SDK
                 MsiSupport.obj
!endif

SRCS		= $(TOBJS:.obj=.cpp) \
                  $(OBJS:.obj=.cpp)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

!ifdef HAS_MSI_SDK
LINKWITH        = $(LIBS) msi.lib
!else
LINKWITH        = $(LIBS)
!endif
TLINKWITH 	= $(LIBS) iceserviceinstaller$(LIBSUFFIX).lib

!ifndef BUILD_UTILS

CPPFLAGS	= $(CPPFLAGS) -DICE_SERVICE_INSTALLER_API_EXPORTS

!endif

!if "$(CPP_COMPILER)" != "BCC2006" && "$(OPTIMIZE)" != "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
TPDBFLAGS       = /pdb:$(TOOL:.exe=.pdb)
!endif

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS)
	$(LINK) $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LINKWITH)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

$(TOOL): $(TOBJS)
	$(LINK) $(LD_EXEFLAGS) $(TPDBFLAGS) $(TOBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(TLINKWITH)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest security.manifest -outputresource:$@;#1 && del /q $@.manifest

!ifdef BUILD_UTILS

clean::
	del /q $(TOOL:.exe=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)
	copy $(TOOL) $(install_bindir)

!if "$(OPTIMIZE)" != "yes"

!if "$(CPP_COMPILER)" == "BCC2006"

install:: all
	copy $(DLLNAME:.dll=.tds) $(install_bindir)
	copy $(SVCDLLNAME:.dll=.tds) $(install_bindir)
	copy $(TOOL:.exe=.tds) $(install_bindir)

!else

install:: all
	copy $(DLLNAME:.dll=.pdb) $(install_bindir)
	copy $(SVCDLLNAME:.dll=.pdb) $(install_bindir)
	copy $(TOOL:.exe=.pdb) $(install_bindir)

!endif

!endif

!else

install:: all

$(EVERYTHING)::
	@$(MAKE) -nologo /f Makefile.mak BUILD_UTILS=1 $@

!endif

!include .depend

