# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= IceStorm
LIBNAME		= $(PKG).dll
TARGETS		= $(bindir)\$(LIBNAME)
POLICY_TARGET   = $(POLICY).dll

SRCS		= AssemblyInfo.cs

GEN_SRCS	= $(GDIR)\IceStorm.cs

SDIR		= $(slicedir)\IceStorm
GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(TARGETS) -warnaserror-
MCSFLAGS	= $(MCSFLAGS) -keyfile:$(KEYFILE)
MCSFLAGS	= $(MCSFLAGS) /doc:$(bindir)\$(PKG).xml /nowarn:1591

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I$(slicedir) --ice

$(TARGETS):: $(SRCS) $(GEN_SRCS)
	$(MCS) /baseaddress:0x21000000 $(MCSFLAGS) -r:$(refdir)\Ice.dll $(SRCS) $(GEN_SRCS)

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
