# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe testservice.dll

C_SRCS		= Client.cs AllTests.cs
S_SRCS		= TestI.cs TestServiceI.cs

GEN_SRCS	= $(GDIR)\Test.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

SLICE2CSFLAGS	= $(SLICE2CSFLAGS) -I. -I"$(slicedir)"

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" $(C_SRCS) $(GEN_SRCS)

testservice.dll: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -target:library -out:$@ -r:"$(refdir)\IceBox.dll" -r:"$(refdir)\Ice.dll" \
		$(S_SRCS) $(GEN_SRCS)
