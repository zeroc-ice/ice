# Copyright (c) ZeroC, Inc.

$(project)_libraries    := Glacier2

Glacier2_targetdir              := $(libdir)
Glacier2_dependencies           := Ice
Glacier2_cppflags               := $(api_exports_cppflags)
Glacier2_sliceflags             := --include-dir Glacier2

projects += $(project)
