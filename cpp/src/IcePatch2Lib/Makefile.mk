# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_libraries    := IcePatch2

IcePatch2_targetdir     := $(libdir)
IcePatch2_dependencies  := Ice
IcePatch2_libs          := bz2
IcePatch2_sliceflags    := --include-dir IcePatch2
IcePatch2_cppflags      := $(nodeprecatedwarnings-cppflags)

projects += $(project)
