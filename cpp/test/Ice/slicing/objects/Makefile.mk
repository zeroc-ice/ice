# Copyright (c) ZeroC, Inc.

$(project)_client_sources          = $(test-client-sources) ClientPrivate.ice
$(project)_server_sources          = $(test-server-sources) ServerPrivate.ice
$(project)_serveramd_sources       = $(test-serveramd-sources) ServerPrivate.ice

tests += $(project)
