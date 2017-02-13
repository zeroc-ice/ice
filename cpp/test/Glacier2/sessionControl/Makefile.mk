# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_dependencies = Glacier2 Ice TestCommon

$(test)_client_sources = Client.cpp Session.ice

$(test)_server_sources = Server.cpp SessionI.cpp Session.ice

tests += $(test)
