# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_client_sources 	= Client.m AllTests.m SlicingObjectsTestClient.ice SlicingObjectsForwardClient.ice
$(test)_server_sources 	= Server.m TestI.m SlicingObjectsTestServer.ice SlicingObjectsForwardServer.ice

tests += $(test)
