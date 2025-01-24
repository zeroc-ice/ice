# Copyright (c) ZeroC, Inc.

$(project)_libraries    = IceDB

IceDB_targetdir         := $(libdir)
IceDB_dependencies      := Ice
IceDB_libs              := lmdb
IceDB_cppflags          := -DICE_DB_API_EXPORTS $(api_exports_cppflags)
IceDB_devinstall        := no

projects += $(project)
