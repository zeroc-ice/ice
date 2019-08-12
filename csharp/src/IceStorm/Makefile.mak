# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= IceStorm
LIBNAME		= $(PKG).dll
TARGETS		= $(assembliesdir)\$(LIBNAME)
POLICY_TARGET   = $(POLICY).dll

SRCS		= AssemblyInfo.cs

GEN_SRCS	= $(GDIR)\IceStorm.cs \
		$(GDIR)\Metrics.cs

SDIR		= $(slicedir)\IceStorm
GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	= $(MCSFLAGS) -keyfile:"$(KEYFILE)"
MCSFLAGS	= $(MCSFLAGS) /doc:$(assembliesdir)\$(PKG).xml /nowarn:1591

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I$(slicedir) --ice

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) /baseaddress:0x21000000 $(MCSFLAGS) -r:$(refdir)\Ice.dll $(SRCS) $(GEN_SRCS)
	@if defined SIGN_CERTIFICATE echo ^ ^ ^ Signing $@ && \
		signtool sign /f "$(SIGN_CERTIFICATE)" /p $(SIGN_PASSWORD) /t $(SIGN_TIMESTAMPSERVER) $@

!if "$(DEBUG)" == "yes"
clean::
	del /q $(assembliesdir)\$(PKG).pdb
!endif

clean::
	del /q $(assembliesdir)\$(PKG).xml

install:: all
	copy $(assembliesdir)\$(LIBNAME) "$(install_assembliesdir)"
	copy $(assembliesdir)\$(PKG).xml "$(install_assembliesdir)"
!if "$(generate_policies)" == "yes"
        copy $(assembliesdir)\$(POLICY) "$(install_assembliesdir)"
	copy $(assembliesdir)\$(POLICY_TARGET) "$(install_assembliesdir)"
!endif
!if "$(DEBUG)" == "yes"
	copy $(assembliesdir)\$(PKG).pdb "$(install_assembliesdir)"
!endif
