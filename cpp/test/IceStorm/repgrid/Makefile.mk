# Copyright (c) ZeroC, Inc.

$(project)_dependencies    = IceStorm Ice TestCommon

$(project)_client_sources  = Client.cpp Single.ice

tests += $(project)
