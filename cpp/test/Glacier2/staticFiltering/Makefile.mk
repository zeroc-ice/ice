# Copyright (c) ZeroC, Inc.

$(project)_client_sources = Client.cpp BackendI.cpp Backend.ice
$(project)_client_dependencies = Glacier2

$(project)_server_sources = Server.cpp BackendI.cpp Backend.ice

tests += $(project)
