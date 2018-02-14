# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ../../..

TARGETS		= client.exe plugins/Plugin.dll

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

include $(top_srcdir)/config/Make.rules.cs

client.exe: $(C_SRCS)
	$(MCS) $(MCSFLAGS) -target:exe -out:$@ $(call ref,Ice) $(C_SRCS)

plugins/Plugin.dll: $(P_SRCS)
	$(MCS) $(MCSFLAGS) -target:library -out:plugins/Plugin.dll $(call ref,Ice) /keyfile:$(KEYFILE) $(P_SRCS)

clean::
	rm -f plugins/Plugin.dll
