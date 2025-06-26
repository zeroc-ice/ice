# Copyright (c) ZeroC, Inc.

$(project)_programs       :=  icegriddb

icegriddb_targetdir       := $(bindir)
icegriddb_dependencies    := Ice Glacier2 IceGrid
icegriddb_libs            := lmdb
icegriddb_cppflags        := $(if $(lmdb_includedir),-I$(lmdb_includedir))
icegriddb_sources         := $(addprefix $(currentdir)/,IceGridDB.cpp ../IceDB/IceDB.cpp DBTypes.ice)

projects += $(project)
