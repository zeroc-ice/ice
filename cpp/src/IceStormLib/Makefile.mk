# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries    := IceStorm

IceStorm_targetdir      := $(libdir)
IceStorm_dependencies   := Ice
IceStorm_sliceflags     := --include-dir IceStorm

projects += $(project)
