# Copyright (c) ZeroC, Inc.

# The IceUtil is a static library with a subset of the Ice library, used to build the Slice compilers.

$(project)_libraries    = IceUtil

IceUtil_targetdir       := $(libdir)
IceUtil_cppflags        := $(if $(filter yes,$(libbacktrace)),-DICE_LIBBACKTRACE)

IceUtil_extra_sources   := src/Ice/ConsoleUtil.cpp \
                           src/Ice/CtrlCHandler.cpp \
                           src/Ice/Demangle.cpp \
                           src/Ice/Exception.cpp \
                           src/Ice/FileUtil.cpp \
                           src/Ice/LocalException.cpp \
                           src/Ice/Options.cpp \
                           src/Ice/OutputUtil.cpp \
                           src/Ice/Random.cpp \
                           src/Ice/StringConverter.cpp \
                           src/Ice/StringUtil.cpp \
                           src/Ice/UUID.cpp

# Always enable the static configuration for the IceUtil library and never
# install it.
IceUtil_always_enable_configs   := static
IceUtil_always_enable_platforms := $(build-platform)
IceUtil_install_configs         := none

projects += $(project)
