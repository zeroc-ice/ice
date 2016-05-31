# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries	:= IcePatch2

IcePatch2_targetdir	:= $(libdir)
IcePatch2_dependencies	:= Ice
IcePatch2_libs		:= bz2
IcePatch2_sliceflags	:= --include-dir IcePatch2 --dll-export ICE_PATCH2_API
IcePatch2_cppflags	:= $(nodeprecatedwarnings-cppflags)

projects += $(project)
