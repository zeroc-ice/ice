# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_sliceflags      = --checksum
$(test)_client_sources  = $(test-client-sources) ClientTypes.ice
$(test)_server_sources  = $(test-server-sources) ServerTypes.ice

tests += $(test)
