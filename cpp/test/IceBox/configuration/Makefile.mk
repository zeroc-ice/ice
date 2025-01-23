# Copyright (c) ZeroC, Inc.

$(project)_libraries := $(project)_TestService

$(project)_TestService_sources             = TestI.cpp Service.cpp Test.ice
$(project)_TestService_dependencies        = IceBox
$(project)_TestService_version             =
$(project)_TestService_soversion           =

tests += $(project)
