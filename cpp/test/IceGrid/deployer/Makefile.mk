# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

$(test)_libraries := $(test)_TestService

$(test)_TestService_sources      = Service.cpp TestI.cpp Test.ice
$(test)_TestService_dependencies = IceBox
$(test)_TestService_version      =
$(test)_TestService_soversion    =

$(test)_client_dependencies = IceGrid Glacier2

tests += $(test)
