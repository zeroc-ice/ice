#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_client_sources = Test.ice TestI.cpp Client.cpp AllTests.cpp
$(test)_dependencies = TestCommon IceSSL Ice

tests += $(test)
