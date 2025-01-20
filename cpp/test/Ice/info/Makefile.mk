# Copyright (c) ZeroC, Inc.

$(test)_client_sources = Test.ice TestI.cpp Client.cpp AllTests.cpp
$(test)_dependencies = TestCommon Ice

tests += $(test)
