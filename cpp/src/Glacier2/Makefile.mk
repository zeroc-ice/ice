# Copyright (c) ZeroC, Inc.

$(project)_programs             := glacier2router
$(project)_dependencies         := Glacier2 Ice Glacier2CryptPermissionsVerifier

glacier2router_targetdir        := $(bindir)

projects += $(project)
