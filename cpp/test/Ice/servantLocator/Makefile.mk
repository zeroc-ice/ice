# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_client_sources = $(test-client-sources) ServantLocatorI.cpp
$(test)_server_sources = $(test-server-sources) ServantLocatorI.cpp
$(test)_serveramd_sources = $(test-serveramd-sources) ServantLocatorI.cpp

tests += $(test)
