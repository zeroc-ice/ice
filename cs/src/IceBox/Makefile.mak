# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

PKG		= IceBox
LIBNAME		= $(bindir)\$(PKG).dll
ICEBOXNET	= $(bindir)\iceboxnet.exe
TARGETS		= $(LIBNAME) $(ICEBOXNET)
POLICY_TARGET   = $(POLICY).dll

L_SRCS		= AssemblyInfo.cs
I_SRCS		= AssemblyInfoExe.cs Server.cs ServiceManagerI.cs

GEN_SRCS	= $(GDIR)\IceBox.cs

SDIR		= $(slicedir)\IceBox
GDIR		= generated

!include $(top_srcdir)/config/Make.rules.mak.cs

EXE_MCSFLAGS	= $(MCSFLAGS) -target:exe

LIB_MCSFLAGS	= $(MCSFLAGS) -target:library -out:$(LIBNAME)
LIB_MCSFLAGS	= $(LIB_MCSFLAGS) -keyfile:$(KEYFILE)
LIB_MCSFLAGS	= $(LIB_MCSFLAGS) /doc:$(bindir)\$(PKG).xml /nowarn:1591

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) --checksum --ice -I. -I$(slicedir)

$(ICEBOXNET): $(I_SRCS) $(LIBNAME)
	$(MCS) $(EXE_MCSFLAGS) -out:$@ -r:$(LIBNAME) -r:$(refdir)\Ice.dll $(I_SRCS)

$(LIBNAME): $(L_SRCS) $(GEN_SRCS)
	$(MCS) /baseaddress:0x25000000 $(LIB_MCSFLAGS) -r:$(refdir)\Ice.dll $(L_SRCS) $(GEN_SRCS)

!if "$(DEBUG)" == "yes"
clean::
	del /q $(bindir)\$(PKG).pdb
	del /q $(bindir)\iceboxnet.pdb
!endif

clean::
	del /q $(bindir)\$(PKG).xml


install:: all
	copy $(LIBNAME) "$(install_bindir)"
	copy $(bindir)\$(PKG).xml "$(install_bindir)"
!if "$(generate_policies)" == "yes"
	copy $(bindir)\$(POLICY) "$(install_bindir)"
	copy $(bindir)\$(POLICY_TARGET) "$(install_bindir)"
!endif
!if "$(DEBUG)" == "yes"
	copy $(bindir)\$(PKG).pdb "$(install_bindir)"
!endif

install:: all
	copy $(ICEBOXNET) "$(install_bindir)"
!if "$(DEBUG)" == "yes"
	copy $(bindir)\iceboxnet.pdb "$(install_bindir)"
!endif

!include .depend.mak
