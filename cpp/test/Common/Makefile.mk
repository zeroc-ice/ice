# Copyright (c) ZeroC, Inc.

$(project)_libraries    = TestCommon
$(project)_caninstall   := no

#
# Put the shared TestCommon library in the lib directory for convenience on platforms
# which don't support something like @loader_path. We don't use lib or $(libdir) here
# because TestCommon isn't installed with the binary distribution.
#
TestCommon[shared]_targetdir    := $(call mappingdir,$(currentdir),lib)
TestCommon_dependencies         := Ice
TestCommon_cppflags             := -DTEST_API_EXPORTS $(api_exports_cppflags) -I$(includedir) -Itest/include

projects += $(project)
