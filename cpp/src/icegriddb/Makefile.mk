# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_programs             :=  icegriddb
$(project)_generated_includedir := $(project)/generated/IceGrid
$(project)_sliceflags           := -Isrc --include-dir IceGrid

$(project)/IceGridDB.cpp: $(includedir)/generated/IceGrid/Admin.h
$(project)/generated/DBTypes.cpp: $(includedir)/generated/IceGrid/Admin.h

icegriddb_targetdir       := $(bindir)
icegriddb_dependencies    := Ice IceDB Glacier2
icegriddb_cppflags	  := $(if $(lmdb_includedir),-I$(lmdb_includedir))
icegriddb_sources         := $(addprefix $(slicedir)/IceGrid/,Descriptor.ice Exception.ice) \
                             $(addprefix $(currentdir)/,IceGridDB.cpp DBTypes.ice)

projects += $(project)
