# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_client_sources = $(test-client-sources) ServantLocatorI.cpp
$(test)_server_sources = $(test-server-sources) ServantLocatorI.cpp
$(test)_serveramd_sources = $(test-server-sources) ServantLocatorI.cpp

tests += $(test)