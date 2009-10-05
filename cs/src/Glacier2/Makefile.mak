# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= Glacier2
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\$(LIBNAME)
POLICY_TARGET   = $(POLICY).dll

SRCS		= SessionFactoryHelper.cs \
		  SessionHelper.cs \
		  SessionCallback.cs \
		  Application.cs \
		  AssemblyInfo.cs

GEN_SRCS	= $(GDIR)\PermissionsVerifier.cs \
		  $(GDIR)\Router.cs \
		  $(GDIR)\Session.cs \
		  $(GDIR)\SSLInfo.cs

SDIR		= $(slicedir)\Glacier2
GDIR		= generated

!include $(top_srcdir)/config/Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	= $(MCSFLAGS) -keyfile:$(KEYFILE)
MCSFLAGS	= $(MCSFLAGS) /doc:$(bindir)\$(PKG).xml /nowarn:1591 -r:WindowsBase.dll

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I$(slicedir)

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -r:$(refdir)\Ice.dll $(SRCS) $(GEN_SRCS)

!if "$(DEBUG)" == "yes"
clean::
	del /q $(bindir)\$(PKG).pdb
!endif

install:: all
	copy $(bindir)\$(LIBNAME) $(install_bindir)
	copy $(bindir)\$(PKG).xml $(install_bindir)
	copy $(bindir)\$(POLICY) $(install_bindir)
	copy $(bindir)\$(POLICY_TARGET) $(install_bindir)
!if "$(DEBUG)" == "yes"
	copy $(bindir)\$(PKG).pdb $(install_bindir)
!endif

!include .depend
