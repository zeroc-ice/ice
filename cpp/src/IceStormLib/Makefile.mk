#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    := IceStorm

IceStorm_targetdir      := $(libdir)
IceStorm_dependencies   := Ice
IceStorm_sliceflags     := --include-dir IceStorm

projects += $(project)
