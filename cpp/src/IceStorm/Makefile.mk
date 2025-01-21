# Copyright (c) ZeroC, Inc.

$(project)_libraries            := IceStormService
$(project)_programs             := icestormadmin icestormdb
$(project)_dependencies         := IceStorm Ice Glacier2

IceStormService_targetdir       := $(libdir)
IceStormService_dependencies    := IceGrid IceBox IceDB
IceStormService_cppflags        := $(if $(lmdb_includedir),-I$(lmdb_includedir))
IceStormService_devinstall      := no
IceStormService_sources         := $(addprefix $(currentdir)/,Instance.cpp \
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
                                                             LinkRecord.ice \
                                                             LLURecord.ice \
                                                             SubscriberRecord.ice \
                                                             DBTypes.ice)

icestormadmin_targetdir         := $(bindir)
icestormadmin_sources           := $(addprefix $(currentdir)/,Admin.cpp \
                                                             Grammar.y \
                                                             Parser.cpp \
                                                             Scanner.l \
                                                             LLURecord.ice \
                                                             SubscriberRecord.ice \
                                                             Election.ice \
                                                             IceStormInternal.ice)

icestormdb_targetdir            := $(bindir)
icestormdb_dependencies         := IceDB
icestormdb_cppflags             := $(if $(lmdb_includedir),-I$(lmdb_includedir))
icestormdb_sources              := $(addprefix $(currentdir)/,IceStormDB.cpp SubscriberRecord.ice DBTypes.ice LLURecord.ice)

projects += $(project)
