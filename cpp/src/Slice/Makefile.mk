# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	:= Slice

Slice_targetdir		:= $(libdir)
Slice_libs		:= mcpp

# Always enable the static configuration for the Slice library and never
# install it
Slice_always_enable_configs 	:= static
Slice_always_enable_platforms	:= $(build-platform)
Slice_install_configs	    	:= none
Slice_bisonflags		:= --name-prefix "slice_"

projects += $(project)
