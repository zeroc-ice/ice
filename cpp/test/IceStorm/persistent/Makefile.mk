# Copyright (c) ZeroC, Inc.

$(project)_programs        = client
$(project)_dependencies    = IceStorm Ice TestCommon

$(project)_client_sources  = Client.cpp

tests += $(project)
