# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_programs        = client
$(test)_dependencies    = IceStorm Ice TestCommon

$(test)_client_sources  = Client.cpp

tests += $(test)
