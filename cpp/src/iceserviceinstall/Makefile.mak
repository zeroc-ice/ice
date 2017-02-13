# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

TOOL		= $(top_srcdir)\bin\iceserviceinstall.exe

TARGETS         = $(TOOL)

OBJS            = .\ServiceInstaller.obj \
                  .\Install.obj

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

LINKWITH	= /MANIFEST /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'" \
                  authz.lib advapi32.lib $(LIBS)

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS       = /pdb:$(TOOL:.exe=.pdb)
!endif

RES_FILE        = IceServiceInstall.res

$(TOOL): $(OBJS) IceServiceInstall.res
	$(LINK) $(LD_EXEFLAGS) $(PDBFLAGS) $(OBJS) $(SETARGV) $(PREOUT)$@ $(PRELIBS)$(LINKWITH) $(RES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest $(EXTRA_MANIFEST) -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

clean::
	del /q $(TOOL:.exe=.*)
	del /q IceServiceInstall.res

install:: all
	copy $(TOOL) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(TOOL:.exe=.pdb) "$(install_bindir)"

!endif
