# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_libraries := $(test)_TestService $(test)_RegistryPlugin

$(test)_TestService_sources 	 	= Service.cpp TestI.cpp Test.ice
$(test)_TestService_dependencies 	= IceBox

$(test)_RegistryPlugin_sources 	 	= RegistryPlugin.cpp
$(test)_RegistryPlugin_dependencies 	= IceGrid Glacier2

$(test)_client_dependencies 		= IceGrid Glacier2

tests += $(test)