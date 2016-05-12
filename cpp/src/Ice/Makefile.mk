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
Ice_cppflags  		= -DICE_API_EXPORTS

ifeq ($(DEFAULT_MUTEX_PROTOCOL), PrioInherit)
    Ice_cppflags        += -DICE_PRIO_INHERIT
endif

ifeq ($(libbacktrace),yes)
    Ice_cppflags        += -DICE_LIBBACKTRACE
endif

Ice_sliceflags		:= --include-dir Ice --dll-export ICE_API
Ice_libs		:= bz2
Ice_system_libs		:= $(ICE_OS_LIBS)
Ice_extra_sources       := $(wildcard src/IceUtil/*.cpp)
Ice_excludes		:= $(currentdir)/DLLMain.cpp

projects += $(project)
