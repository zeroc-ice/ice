# Copyright (c) ZeroC, Inc.

$(project)_libraries    := Glacier2

Glacier2_targetdir              := $(libdir)
Glacier2_dependencies           := Ice
Glacier2_cppflags               := -DGLACIER2_API_EXPORTS
Glacier2_sliceflags             := --include-dir Glacier2

projects += $(project)
