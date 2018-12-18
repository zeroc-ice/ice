# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_programs             := glacier2router
$(project)_sliceflags           := --include-dir Glacier2
$(project)_generated_includedir := $(project)/generated/Glacier2
$(project)_dependencies         := Glacier2 IceSSL Ice

glacier2router_targetdir        := $(bindir)

projects += $(project)
