# Copyright (c) ZeroC, Inc.

$(project)_client_sources          = Client.cpp Forward.ice Test.ice Derived.ice DerivedEx.ice TestI.cpp AllTests.cpp
$(project)_server_sources          = Server.cpp Forward.ice Test.ice Derived.ice DerivedEx.ice TestI.cpp TestIntfI.cpp
$(project)_collocated_sources      = Collocated.cpp Forward.ice Test.ice Derived.ice DerivedEx.ice TestI.cpp TestIntfI.cpp AllTests.cpp

tests += $(project)
