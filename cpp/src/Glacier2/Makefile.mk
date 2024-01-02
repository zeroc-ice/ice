#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_programs             := glacier2router
$(project)_sliceflags           := --include-dir Glacier2
$(project)_generated_includedir := $(project)/generated/Glacier2
$(project)_dependencies         := Glacier2 IceSSL Ice
$(project)_excludes             = src/Glacier2/Instrumentation.ice

glacier2router_targetdir        := $(bindir)

projects += $(project)
