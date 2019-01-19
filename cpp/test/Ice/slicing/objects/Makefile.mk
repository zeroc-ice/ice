#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_sliceflags              := -I$(test)
$(test)_client_sources          = $(test-client-sources) ClientPrivate.ice
$(test)_server_sources          = $(test-server-sources) ServerPrivate.ice
$(test)_serveramd_sources       = $(test-serveramd-sources) ServerPrivateAMD.ice

tests += $(test)
