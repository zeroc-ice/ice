# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_client_sources = $(test-client-sources) ExceptionsI.cpp
$(test)_server_sources = $(test-server-sources) ExceptionsI.cpp
$(test)_serveramd_sources = $(test-serveramd-sources) ExceptionsI.cpp

tests += $(test)
