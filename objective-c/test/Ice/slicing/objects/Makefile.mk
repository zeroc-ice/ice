# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_client_sources  = Client.m AllTests.m SlicingObjectsTestClient.ice SlicingObjectsForwardClient.ice
$(test)_server_sources  = Server.m TestI.m SlicingObjectsTestServer.ice SlicingObjectsForwardServer.ice

tests += $(test)
