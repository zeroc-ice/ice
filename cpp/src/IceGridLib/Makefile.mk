# Copyright (c) ZeroC, Inc.

$(project)_libraries    := IceGrid

IceGrid_targetdir       := $(libdir)
IceGrid_dependencies    := Glacier2 Ice
IceGrid_sliceflags      := --include-dir IceGrid

projects += $(project)
