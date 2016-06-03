# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	= Ice

Ice_targetdir		:= $(libdir)
Ice_cppflags  		= -DICE_API_EXPORTS $(IceUtil_cppflags)

ifeq ($(DEFAULT_MUTEX_PROTOCOL), PrioInherit)
    Ice_cppflags        += -DICE_PRIO_INHERIT
endif

Ice_sliceflags		:= --include-dir Ice --dll-export ICE_API
Ice_libs		:= bz2
Ice_extra_sources       := $(wildcard src/IceUtil/*.cpp)
Ice_excludes		= src/Ice/DLLMain.cpp

ifeq ($(os),Darwin)
Ice_excludes            += src/IceUtil/ConvertUTF.cpp src/IceUtil/Unicode.cpp
endif

Ice[iphoneos]_extra_sources 		:= $(wildcard $(addprefix $(currentdir)/ios/,*.cpp *.mm))
Ice[iphoneos]_excludes	 		:= $(currentdir)/RegisterPluginsInit.cpp
Ice[iphonesimulator]_extra_sources	= $(Ice[iphoneos]_extra_sources)
Ice[iphonesimulator]_excludes	 	= $(Ice[iphoneos]_excludes)

projects += $(project)
