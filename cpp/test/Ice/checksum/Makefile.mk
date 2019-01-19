#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_sliceflags      = --checksum
$(test)_client_sources  = $(test-client-sources) ClientTypes.ice
$(test)_server_sources  = $(test-server-sources) ServerTypes.ice

tests += $(test)
