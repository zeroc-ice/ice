# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_libraries    := IceGrid

IceGrid_targetdir       := $(libdir)
IceGrid_dependencies    := Glacier2 Ice
IceGrid_sliceflags      := --include-dir IceGrid

projects += $(project)
