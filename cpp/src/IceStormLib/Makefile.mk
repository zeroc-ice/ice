# Copyright (c) ZeroC, Inc.

$(project)_libraries    := IceStorm

IceStorm_targetdir      := $(libdir)
IceStorm_dependencies   := Ice
IceStorm_sliceflags     := --include-dir IceStorm

projects += $(project)
