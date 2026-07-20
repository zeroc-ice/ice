# Copyright (c) ZeroC, Inc.

$(project)_client_sources = Client.cpp CallbackI.cpp Callback.ice
$(project)_client_dependencies = Glacier2

ifeq ($(os),Linux)
    $(project)_client_ldflags = -lcrypt
endif

$(project)_server_sources = Server.cpp CallbackI.cpp Callback.ice

tests += $(project)
