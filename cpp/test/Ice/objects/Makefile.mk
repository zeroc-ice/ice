#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_sliceflags              := -I$(test)
$(test)_client_sources          = Client.cpp Forward.ice Test.ice Derived.ice DerivedEx.ice TestI.cpp AllTests.cpp
$(test)_server_sources          = Server.cpp Forward.ice Test.ice Derived.ice DerivedEx.ice TestI.cpp TestIntfI.cpp
$(test)_collocated_sources      = Collocated.cpp Forward.ice Test.ice Derived.ice DerivedEx.ice TestI.cpp TestIntfI.cpp AllTests.cpp

tests += $(test)
