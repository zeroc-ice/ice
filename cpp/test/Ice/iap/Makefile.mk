# Copyright (c) ZeroC, Inc.

# The iAP transport is only available on iOS.
$(project)_platforms      = iphonesimulator iphoneos
$(project)_client_sources = Client.cpp AllTests.cpp Transceiver.mm
$(project)_client_ldflags = -framework ExternalAccessory
$(project)_dependencies   = TestCommon Ice

tests += $(project)
