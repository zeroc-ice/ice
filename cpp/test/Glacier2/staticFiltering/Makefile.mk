#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_client_sources = Client.cpp BackendI.cpp Backend.ice
$(test)_client_dependencies = Glacier2

$(test)_server_sources = Server.cpp BackendI.cpp Backend.ice

tests += $(test)
