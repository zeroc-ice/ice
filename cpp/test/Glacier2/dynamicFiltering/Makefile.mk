# Copyright (c) ZeroC, Inc.

$(project)_dependencies = Glacier2 Ice TestCommon

$(project)_client_sources = Client.cpp Test.ice

$(project)_server_sources = Server.cpp TestControllerI.cpp BackendI.cpp SessionI.cpp Test.ice

tests += $(project)
