# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\glacier2$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\glacier2$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS 	= $(LIBNAME) $(DLLNAME)

SLICE_OBJS	= .\Metrics.obj \
		  .\PermissionsVerifierF.obj \
		  .\PermissionsVerifier.obj \
		  .\Router.obj \
		  .\RouterF.obj \
		  .\Session.obj \
		  .\SSLInfo.obj

OBJS		= .\Application.obj \
		  .\NullPermissionsVerifier.obj \
		  .\SessionHelper.obj \
		  $(SLICE_OBJS)

HDIR		= $(headerdir)\Glacier2
SDIR		= $(slicedir)\Glacier2

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DGLACIER2_API_EXPORTS -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

RES_FILE        = Glacier2.res

SLICE2CPPFLAGS	= --ice --include-dir Glacier2 --dll-export GLACIER2_API $(SLICE2CPPFLAGS)

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) Glacier2.res $(HDIR)\PermissionsVerifierF.h $(HDIR)\RouterF.h
	$(LINK) $(BASE):0x27000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS)$(LIBS) $(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
		$(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

clean::
	-del /q PermissionsVerifierF.cpp $(HDIR)\PermissionsVerifierF.h
	-del /q PermissionsVerifier.cpp $(HDIR)\PermissionsVerifier.h
	-del /q RouterF.cpp $(HDIR)\RouterF.h
	-del /q Router.cpp $(HDIR)\Router.h
	-del /q Session.cpp $(HDIR)\Session.h
	-del /q SSLInfo.cpp $(HDIR)\SSLInfo.h
	-del /q Metrics.cpp $(HDIR)\Metrics.h
	-del /q Glacier2.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif
