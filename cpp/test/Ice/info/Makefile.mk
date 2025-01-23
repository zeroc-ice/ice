# Copyright (c) ZeroC, Inc.

$(project)_client_sources = Test.ice TestI.cpp Client.cpp AllTests.cpp
$(project)_dependencies = TestCommon Ice

tests += $(project)
