# Copyright (c) ZeroC, Inc.

$(project)_client_sources = $(test-client-sources) ServantLocatorI.cpp
$(project)_server_sources = $(test-server-sources) ServantLocatorI.cpp
$(project)_serveramd_sources = $(test-serveramd-sources) ServantLocatorI.cpp

tests += $(project)
