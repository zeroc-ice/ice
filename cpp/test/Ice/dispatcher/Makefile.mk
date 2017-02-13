# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_client_sources = $(test-client-sources) Dispatcher.cpp
$(test)_server_sources = $(test-server-sources) Dispatcher.cpp

tests += $(test)