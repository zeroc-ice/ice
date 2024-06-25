#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = IceUtil

IceUtil_targetdir       := $(libdir)
IceUtil_cppflags        := $(if $(filter yes,$(libbacktrace)),-DICE_LIBBACKTRACE)

IceUtil_extra_sources   := src/Ice/CtrlCHandler.cpp src/Ice/StringConverter.cpp src/StringUtil.cpp src/Ice/UtilException.cpp src/Ice/UUID.cpp

# Always enable the static configuration for the IceUtil library and never
# install it.
IceUtil_always_enable_configs   := static
IceUtil_always_enable_platforms := $(build-platform)
IceUtil_install_configs         := none

projects += $(project)
