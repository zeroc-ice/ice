# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_dependencies    = IceStorm Ice TestCommon

$(test)_client_sources  = Client.cpp Single.ice

tests += $(test)
