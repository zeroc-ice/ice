# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries    = TestCommonObjC
$(project)_noinstall    = 1

#
# Put the shared TestCommon library in the lib directory for convenience on platforms
# which don't support something like @loader_path.
#
TestCommonObjC[shared]_targetdir        := lib
TestCommonObjC_dependencies             := IceObjC
TestCommonObjC_cppflags                 := -DTEST_API_EXPORTS -I$(includedir) -Itest/include

projects += $(project)
