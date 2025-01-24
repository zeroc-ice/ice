# Copyright (c) ZeroC, Inc.

$(project)_libraries    := IceStorm

IceStorm_targetdir      := $(libdir)
IceStorm_dependencies   := Ice
IceStorm_sliceflags     := --include-dir IceStorm
IceStorm_cppflags       := $(api_exports_cppflags)

projects += $(project)
