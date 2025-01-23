# Copyright (c) ZeroC, Inc.

$(project)_libraries := $(project)_TestService $(project)_RegistryPlugin

$(project)_TestService_sources             = Service.cpp TestI.cpp Test.ice
$(project)_TestService_dependencies        = IceBox
$(project)_TestService_version             =
$(project)_TestService_soversion           =

$(project)_RegistryPlugin_sources          = RegistryPlugin.cpp
$(project)_RegistryPlugin_dependencies     = IceGrid Glacier2

$(project)_client_dependencies             = IceGrid Glacier2

tests += $(project)
