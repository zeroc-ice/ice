# Copyright (c) ZeroC, Inc.

$(project)_libraries            := IceStormService
$(project)_programs             := icestormadmin icestormdb
$(project)_dependencies         := IceStorm Ice Glacier2

# lmdb is not necessary for the icestormadmin sources. However, we want to build all objects with the same flags to
# reuse common object files in the different programs.
# we also include api_exports_cppflags because we "export" IceStormService to IceGrid.
$(project)_cppflags             := $(if $(lmdb_includedir),-I$(lmdb_includedir)) $(api_exports_cppflags)

IceStormService_targetdir       := $(libdir)
IceStormService_dependencies    := IceGrid IceBox IceDB
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
icestormdb_sources              := $(addprefix $(currentdir)/,IceStormDB.cpp SubscriberRecord.ice DBTypes.ice LLURecord.ice)

projects += $(project)
