# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe serveramd.exe

C_SRCS		= AllTests.cs Client.cs \
		  ..\..\TestCommon\TestApp.cs
S_SRCS		= TestI.cs Server.cs
SAMD_SRCS	= TestAMDI.cs ServerAMD.cs

GEN_SRCS	= $(GDIR)\Test.cs
GEN_AMD_SRCS	= $(GDIR)\TestAMD.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

!if "$(COMPACT)" != "yes"
SERIAL_DLL	= Serializable.dll
SERIAL_REF	= -r:$(SERIAL_DLL)
!endif

MCSFLAGS	= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I. -I"$(slicedir)" --stream

!if "$(COMPACT)" == "yes"
SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -DCOMPACT
!endif

client.exe: $(C_SRCS) $(GEN_SRCS) $(SERIAL_DLL)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(SERIAL_REF) $(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS) $(SERIAL_DLL)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(SERIAL_REF) $(S_SRCS) $(GEN_SRCS)

serveramd.exe: $(SAMD_SRCS) $(GEN_AMD_SRCS) $(SERIAL_DLL)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(SERIAL_REF) $(SAMD_SRCS) $(GEN_AMD_SRCS)

!if "$(COMPACT)" != "yes"
$(SERIAL_DLL): SerializableClass.cs
	$(MCS) $(MCSFLAGS) -target:library -out:$(SERIAL_DLL) /keyfile:"$(KEYFILE)" SerializableClass.cs

clean::
	del /q $(SERIAL_DLL)
!endif

!include .depend.mak
