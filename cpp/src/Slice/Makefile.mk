# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	:= Slice

Slice_targetdir		:= $(libdir)
Slice_libs		:= mcpp

# Always enable the static configuration for the Slice library
Slice_always_enable_configs := static

projects += $(project)
