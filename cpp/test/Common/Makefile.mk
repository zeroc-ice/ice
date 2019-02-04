#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = TestCommon
$(project)_caninstall   := no

#
# Put the shared TestCommon library in the lib directory for convenience on platforms
# which don't support something like @loader_path. We don't use lib or $(libdir) here
# because TestCommon isn't installed with the binary distribution and it might also
# be used by a component that has Ice as a sub-module (e.g. Freeze).
#
TestCommon[shared]_targetdir    := $(call mappingdir,$(currentdir),lib)
TestCommon_dependencies         := IceSSL Ice
TestCommon_cppflags             := -DTEST_API_EXPORTS -I$(includedir) -I$(project) -Itest/include

projects += $(project)
