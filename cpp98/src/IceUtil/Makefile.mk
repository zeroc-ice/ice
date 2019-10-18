#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = IceUtil

IceUtil_targetdir       := $(libdir)
IceUtil_cppflags        := $(if $(filter yes,$(libbacktrace)),-DICE_LIBBACKTRACE)

ifeq ($(os),Darwin)
IceUtil_excludes        = src/IceUtil/ConvertUTF.cpp src/IceUtil/Unicode.cpp
endif

# Always enable the static configuration for the IceUtil library and never
# install it.
IceUtil_always_enable_configs   := static
IceUtil_always_enable_platforms := $(build-platform)
IceUtil_install_configs         := none

projects += $(project)
