# Copyright (c) ZeroC, Inc.

$(test)_dependencies = Glacier2 Ice TestCommon

$(test)_client_sources = Client.cpp Session.ice

$(test)_server_sources = Server.cpp SessionI.cpp Session.ice

tests += $(test)
