# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe server.exe 

C_SRCS		= Client.cs AllTests.cs 
S_SRCS		= Server.cs TestI.cs 

GEN_SRCS	= $(GDIR)\Test.cs

SDIR		= .

GDIR		= generated

!include $(top_srcdir)\config\Make.rules.mak.cs

MCSFLAGS	= $(MCSFLAGS) -target:exe

client.exe: $(C_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:"$(refdir)\Glacier2.dll" -r:"$(refdir)\IceGrid.dll" \
		$(C_SRCS) $(GEN_SRCS)

server.exe: $(S_SRCS) $(GEN_SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ -r:"$(refdir)\Ice.dll" -r:"$(refdir)\Glacier2.dll" -r:"$(refdir)\IceGrid.dll" \
		$(S_SRCS) $(GEN_SRCS)

clean::
	if exist db\node rmdir /s /q db\node
	if exist db\registry rmdir /s /q db\registry
	if exist db\replica-1 rmdir /s /q db\replica-1
	del /q build.txt

!if "$(OPTIMIZE)" == "yes"

all::
	@echo release > build.txt

!else

all::
	@echo debug > build.txt

!endif