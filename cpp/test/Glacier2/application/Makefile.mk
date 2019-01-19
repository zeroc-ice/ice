#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_client_sources = Client.cpp Callback.ice
$(test)_client_dependencies = Glacier2

$(test)_server_sources = Server.cpp Callback.ice

tests += $(test)
