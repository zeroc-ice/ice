# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries    = Ice

Ice_targetdir           := $(libdir)
Ice_cppflags            = -DICE_API_EXPORTS $(IceUtil_cppflags)
Ice_ldflags             = $(iconv_ldflags)

ifeq ($(DEFAULT_MUTEX_PROTOCOL), PrioInherit)
    Ice_cppflags        += -DICE_PRIO_INHERIT
endif

Ice_sliceflags          := --include-dir Ice
Ice_libs                := bz2
Ice_extra_sources       := $(wildcard src/IceUtil/*.cpp)
Ice_excludes            = src/Ice/DLLMain.cpp

ifeq ($(os),Darwin)
Ice_excludes            += src/IceUtil/ConvertUTF.cpp src/IceUtil/Unicode.cpp
endif

ifeq ($(os),Linux)
ifeq ($(shell pkg-config --exists libsystemd 2> /dev/null && echo yes),yes)
Ice_cppflags                            += -DICE_USE_SYSTEMD $(shell pkg-config --cflags libsystemd)
endif
endif

Ice[iphoneos]_excludes                  := $(wildcard $(addprefix $(currentdir)/,Tcp*.cpp))
Ice[iphoneos]_extra_sources             := $(wildcard $(addprefix $(currentdir)/ios/,*.cpp *.mm))
Ice[iphonesimulator]_excludes           = $(Ice[iphoneos]_excludes)
Ice[iphonesimulator]_extra_sources      = $(Ice[iphoneos]_extra_sources)

projects += $(project)
