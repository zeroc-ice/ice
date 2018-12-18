# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(project)_libraries    = TestCommonObjC
$(project)_caninstall   = no

#
# Put the shared TestCommon library in the lib directory for convenience on platforms
# which don't support something like @loader_path.
#
TestCommonObjC[shared]_targetdir        := lib
TestCommonObjC_dependencies             := IceObjC
TestCommonObjC_cppflags                 := -DTEST_API_EXPORTS -I$(includedir) -Itest/include

projects += $(project)
