# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	= Ice

Ice_targetdir		:= $(libdir)
Ice_dependencies	:= IceUtil
Ice_cppflags  		:= -DICE_API_EXPORTS
Ice_sliceflags		:= --include-dir Ice --dll-export ICE_API
Ice_libs		:= bz2
Ice_system_libs		:= $(ICE_OS_LIBS)
Ice_excludes		:= $(currentdir)/DLLMain.cpp

projects += $(project)
