# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\iceutil$(LIBSUFFIX).lib
DLLNAME 	= $(top_srcdir)\bin\iceutil$(SOVERSION)$(LIBSUFFIX)$(COMPSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

OBJS		= .\ArgVector.obj \
		  .\Cond.obj \
		  .\ConvertUTF.obj \
		  .\CountDownLatch.obj \
		  .\CtrlCHandler.obj \
		  .\Exception.obj \
		  .\FileUtil.obj \
		  .\InputUtil.obj \
		  .\MutexProtocol.obj \
		  .\Options.obj \
		  .\OutputUtil.obj \
		  .\Random.obj \
		  .\RecMutex.obj \
		  .\SHA1.obj \
		  .\Shared.obj \
		  .\StringConverter.obj \
		  .\StringUtil.obj \
		  .\Thread.obj \
		  .\ThreadException.obj \
		  .\Time.obj \
		  .\Timer.obj \
		  .\Unicode.obj \
		  .\UUID.obj

!include $(top_srcdir)/config/Make.rules.mak

CPPFLAGS	= $(CPPFLAGS) -DICE_UTIL_API_EXPORTS -I.. -DWIN32_LEAN_AND_MEAN

!if "$(GENERATE_PDB)" == "yes"
PDBFLAGS	= /pdb:$(DLLNAME:.dll=.pdb)

!endif

RES_FILE	= IceUtil.res

!if "$(STATICLIBS)" == "yes"

$(DLLNAME):

$(LIBNAME): $(OBJS)
	$(AR) $(ARFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ DbgHelp.lib

!else

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(OBJS) IceUtil.res
	$(LINK) $(BASE):0x20000000 $(LD_DLLFLAGS) $(PDBFLAGS) $(OBJS) $(PREOUT)$@ $(PRELIBS) $(ICEUTIL_OS_LIBS) \
		$(RES_FILE)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
		$(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

!endif

clean::
	-del /q IceUtil.res

install:: all
	copy $(LIBNAME) "$(install_libdir)"
	copy $(DLLNAME) "$(install_bindir)"

!if "$(GENERATE_PDB)" == "yes"

install:: all
	copy $(DLLNAME:.dll=.pdb) "$(install_bindir)"

!endif
