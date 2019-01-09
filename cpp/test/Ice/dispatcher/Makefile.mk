# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_client_sources = $(test-client-sources) Dispatcher.cpp
$(test)_server_sources = $(test-server-sources) Dispatcher.cpp

tests += $(test)
