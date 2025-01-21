# Copyright (c) ZeroC, Inc.

$(test)_dependencies    = IceStorm Ice TestCommon

$(test)_client_sources  = Client.cpp Single.ice

tests += $(test)
