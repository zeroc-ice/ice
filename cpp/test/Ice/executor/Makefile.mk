# Copyright (c) ZeroC, Inc.

$(test)_client_sources = $(test-client-sources) Executor.cpp
$(test)_server_sources = $(test-server-sources) Executor.cpp

tests += $(test)
