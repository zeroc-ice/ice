# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= publisher.exe subscriber.exe

C_SRCS		= Publisher.cs
S_SRCS		= Subscriber.cs

GEN_SRCS	= $(GDIR)\Clock.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe -warnaserror-

publisher.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:"$(refdir)\IceStorm.dll" $(C_SRCS) $(GEN_SRCS)

subscriber.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:"$(refdir)\IceStorm.dll" $(S_SRCS) $(GEN_SRCS)

clean::
	if exist db\__Freeze rmdir /q /s db\__Freeze
	for %f in (db\*) do if not %f == db\.gitignore del /q %f

!include .depend.mak
