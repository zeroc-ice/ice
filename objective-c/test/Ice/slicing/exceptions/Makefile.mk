# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_client_sources  = Client.m AllTests.m SlicingExceptionsTestClient.ice
$(test)_server_sources  = Server.m TestI.m SlicingExceptionsTestServer.ice

tests += $(test)
