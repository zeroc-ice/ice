# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..\..

TARGETS		= client.exe plugins\Plugin.dll

C_SRCS		= Client.cs

P_SRCS		= BasePlugin.cs \
		PluginFactory.cs \
		PluginInitializeFailException.cs \
		BasePluginFail.cs \
		PluginInitializeFailFactory.cs \
		PluginOneFactory.cs \
		PluginOneFailFactory.cs \
		PluginThreeFactory.cs \
		PluginThreeFailFactory.cs \
		PluginTwoFactory.cs \
		PluginTwoFailFactory.cs

SDIR		= .

!include $(top_srcdir)\config\Make.rules.mak.cs

client.exe: $(C_SRCS)
	$(MCS) $(MCSFLAGS) -target:exe -out:$@ -r:"$(refdir)\Ice.dll" $(C_SRCS)

plugins\Plugin.dll: $(P_SRCS)
	$(MCS) $(MCSFLAGS) -target:library -out:plugins\Plugin.dll -r:"$(refdir)\Ice.dll" /keyfile:"$(KEYFILE)" $(P_SRCS)

!if "$(DEBUG)" == "yes"
clean::
	del /q plugins\Plugin.pdb
!endif
