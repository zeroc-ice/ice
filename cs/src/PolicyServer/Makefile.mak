# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice for Silverlight  is licensed to you under the terms
# described in the ICESL_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

TARGETS		= $(bindir)\policyserver.exe
TARGETS_CONFIG	= $(TARGETS:.exe=.exe.config)

SRCS		= PolicyServer.cs \
		  AssemblyInfo.cs

!include $(top_srcdir)\config\Make.rules.mak.cs

MCS			= csc -nologo

MCSFLAGS = -warnaserror -d:MAKEFILE_BUILD
!if "$(DEBUG)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -debug -define:DEBUG
!endif

!if "$(OPTIMIZE)" == "yes"
MCSFLAGS 		= $(MCSFLAGS) -optimize+
!endif

MCSFLAGS	= $(MCSFLAGS) -target:exe

$(bindir)\policyserver.exe: $(SRCS)
	$(MCS) $(MCSFLAGS) -out:$@ $(SRCS)

!if "$(DEBUG)" == "yes"
clean::
	del /q $(bindir)\policyserver.pdb
!endif