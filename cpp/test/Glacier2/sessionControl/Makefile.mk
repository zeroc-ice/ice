# Copyright (c) ZeroC, Inc.

$(project)_dependencies = Glacier2 Ice TestCommon

$(project)_client_sources = Client.cpp Session.ice

$(project)_server_sources = Server.cpp SessionI.cpp Session.ice

tests += $(project)
