# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(project)_libraries 		:= IceStormService
$(project)_programs		:= icestormadmin icestormdb
$(project)_sliceflags   	:= -Isrc --include-dir IceStorm
$(project)_generated_includedir := $(project)/generated/IceStorm
$(project)_dependencies		:= IceStorm Ice

IceStormService_targetdir	:= $(libdir)
IceStormService_dependencies 	:= IceGrid Glacier2 IceBox IceDB
IceStormService_cppflags	:= $(if $(lmdb_includedir),-I$(lmdb_includedir))
IceStormService_sources   	:= $(addprefix $(currentdir)/,Instance.cpp \
							     InstrumentationI.cpp \
							     NodeI.cpp \
							     Observers.cpp \
							     Service.cpp \
							     Subscriber.cpp \
							     TopicI.cpp \
							     TopicManagerI.cpp \
							     TraceLevels.cpp \
							     TransientTopicI.cpp \
							     TransientTopicManagerI.cpp \
							     Util.cpp \
							     Election.ice \
							     IceStormInternal.ice \
							     Instrumentation.ice \
							     LinkRecord.ice \
							     LLURecord.ice \
							     SubscriberRecord.ice \
							     DBTypes.ice)

icestormadmin_targetdir		:= $(bindir)
icestormadmin_sources	   	:= $(addprefix $(currentdir)/,Admin.cpp \
							     Grammar.y \
							     Parser.cpp \
							     Scanner.l \
							     LLURecord.ice \
							     SubscriberRecord.ice \
							     Election.ice \
							     IceStormInternal.ice)

icestormdb_targetdir		:= $(bindir)
icestormdb_dependencies 	:= IcePatch2 IceDB
icestormdb_cppflags		:= $(if $(lmdb_includedir),-I$(lmdb_includedir))
icestormdb_sources	   	:= $(addprefix $(currentdir)/,IceStormDB.cpp DBTypes.ice)

projects += $(project)
