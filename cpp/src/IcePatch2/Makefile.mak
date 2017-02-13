# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

SERVER		= $(top_srcdir)\bin\icepatch2server.exe
CLIENT		= $(top_srcdir)\bin\icepatch2client.exe
CALC		= $(top_srcdir)\bin\icepatch2calc.exe

TARGETS         = $(SERVER) $(CLIENT) $(CALC)

SOBJS		= .\FileServerI.obj \
		  .\Server.obj

COBJS		= .\Client.obj

CALCOBJS	= .\Calc.obj

OBJS		= $(SOBJS) \
		  $(COBJS) \
		  $(CALCOBJS)

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I. -I.. $(CPPFLAGS) -DWIN32_LEAN_AND_MEAN

LD_EXEFLAGS     = /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" $(LD_EXEFLAGS)

!if "$(GENERATE_PDB)" == "yes"
SPDBFLAGS       = /pdb:$(SERVER:.exe=.pdb)
CPDBFLAGS       = /pdb:$(CLIENT:.exe=.pdb)
CAPDBFLAGS      = /pdb:$(CALC:.exe=.pdb)
!endif

SRES_FILE       = IcePatch2Server.res
CRES_FILE       = IcePatch2Client.res
CARES_FILE      = IcePatch2Calc.res

$(SERVER): $(SOBJS) IcePatch2Server.res
	$(LINK) $(LD_EXEFLAGS) $(SPDBFLAGS) $(SETARGV) $(SOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib $(SRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(CLIENT): $(COBJS) IcePatch2Client.res
	$(LINK) $(LD_EXEFLAGS) $(CPDBFLAGS) $(SETARGV) $(COBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib $(CRES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

$(CALC): $(CALCOBJS) IcePatch2Calc.res
	$(LINK) $(LD_EXEFLAGS) $(CAPDBFLAGS) $(SETARGV) $(CALCOBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) \
		icepatch2$(LIBSUFFIX).lib $(CARES_FILE)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#1 && del /q $@.manifest
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

clean::
	-del /q $(SERVER:.exe=.*)
	-del /q $(CLIENT:.exe=.*)
	-del /q $(CALC:.exe=.*)
	-del /q IcePatch2Server.res IcePatch2Client.res IcePatch2Calc.res

install:: all
	copy $(SERVER) "$(install_bindir)"
	copy $(CLIENT) "$(install_bindir)"
	copy $(CALC) "$(install_bindir)"


!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(SERVER:.exe=.pdb) "$(install_bindir)"
	copy $(CLIENT:.exe=.pdb) "$(install_bindir)"
	copy $(CALC:.exe=.pdb) "$(install_bindir)"

!endif
