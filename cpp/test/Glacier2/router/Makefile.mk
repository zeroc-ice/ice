# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_client_sources = Client.cpp CallbackI.cpp Callback.ice
$(test)_client_dependencies = Glacier2

$(test)_server_sources = Server.cpp CallbackI.cpp Callback.ice

tests += $(test)
