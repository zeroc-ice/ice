# Copyright (c) ZeroC, Inc.

$(test)_client_sources = Client.cpp CallbackI.cpp Callback.ice
$(test)_client_dependencies = Glacier2

$(test)_server_sources = Server.cpp CallbackI.cpp Callback.ice

tests += $(test)
