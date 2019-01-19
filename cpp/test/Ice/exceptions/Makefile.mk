#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_client_sources = $(test-client-sources) ExceptionsI.cpp
$(test)_server_sources = $(test-server-sources) ExceptionsI.cpp
$(test)_serveramd_sources = $(test-serveramd-sources) ExceptionsI.cpp

tests += $(test)
