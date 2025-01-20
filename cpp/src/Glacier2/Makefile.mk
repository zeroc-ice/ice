# Copyright (c) ZeroC, Inc.

$(project)_programs             := glacier2router
$(project)_sliceflags           := --include-dir Glacier2
$(project)_generated_includedir := $(project)/generated/Glacier2
$(project)_dependencies         := Glacier2 Ice

glacier2router_targetdir        := $(bindir)

projects += $(project)
