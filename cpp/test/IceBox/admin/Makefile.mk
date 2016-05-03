# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_libraries := $(test)_TestService

$(test)_TestService_sources         = TestI.cpp Service.cpp Test.ice
$(test)_TestService_dependencies    = IceBox

tests += $(test)
