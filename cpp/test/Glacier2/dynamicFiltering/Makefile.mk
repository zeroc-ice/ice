# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_dependencies = Glacier2 Ice TestCommon

$(test)_client_sources = Client.cpp Test.ice

$(test)_server_sources = Server.cpp TestControllerI.cpp BackendI.cpp SessionI.cpp Test.ice

tests += $(test)
