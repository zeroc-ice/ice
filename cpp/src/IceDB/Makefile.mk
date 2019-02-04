#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = IceDB

IceDB_targetdir         := $(libdir)
IceDB_dependencies      := Ice
IceDB_libs              := lmdb
IceDB_cppflags          := -DICE_DB_API_EXPORTS
IceDB_devinstall        := no

projects += $(project)
