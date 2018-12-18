# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_sliceflags              := -I$(test)
$(test)_client_sources          = Client.m ObjectsTest.ice ObjectsDerived.ice ObjectsDerivedEx.ice TestI.m AllTests.m
$(test)_server_sources          = Server.m ObjectsTest.ice ObjectsDerived.ice ObjectsDerivedEx.ice TestI.m TestIntfI.m
$(test)_collocated_sources      = Collocated.m ObjectsTest.ice ObjectsDerived.ice ObjectsDerivedEx.ice TestI.m TestIntfI.m AllTests.m

tests += $(test)
