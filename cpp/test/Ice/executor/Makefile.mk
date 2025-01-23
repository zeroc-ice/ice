# Copyright (c) ZeroC, Inc.

$(project)_client_sources = $(test-client-sources) Executor.cpp
$(project)_server_sources = $(test-server-sources) Executor.cpp

tests += $(project)
