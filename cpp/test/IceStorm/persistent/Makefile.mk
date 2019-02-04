#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_programs        = client
$(test)_dependencies    = IceStorm Ice TestCommon

$(test)_client_sources  = Client.cpp

tests += $(test)
