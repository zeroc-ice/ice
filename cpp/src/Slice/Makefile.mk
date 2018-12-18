# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_libraries    := Slice

Slice_targetdir         := $(libdir)
Slice_libs              := mcpp

# Always enable the static configuration for the Slice library and never
# install it
Slice_always_enable_configs     := static
Slice_always_enable_platforms   := $(build-platform)
Slice_install_configs           := none
Slice_bisonflags                := --name-prefix "slice_"

projects += $(project)
