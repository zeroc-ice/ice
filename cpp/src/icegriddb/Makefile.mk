#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_programs             :=  icegriddb
$(project)_sliceflags           := -DICE_BUILDING_ICEGRIDDB

$(project)/IceGridDB.cpp: $(includedir)/generated/IceGrid/Admin.h
$(project)/generated/DBTypes.cpp: $(includedir)/generated/IceGrid/Admin.h

icegriddb_targetdir       := $(bindir)
icegriddb_dependencies    := Ice IceDB Glacier2
icegriddb_cppflags        := $(if $(lmdb_includedir),-I$(lmdb_includedir))
icegriddb_sources         := $(addprefix $(slicedir)/IceGrid/,Descriptor.ice Exception.ice) \
                             $(addprefix $(currentdir)/,IceGridDB.cpp DBTypes.ice)

projects += $(project)
