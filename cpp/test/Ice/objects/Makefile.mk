# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_sliceflags              := -I$(test)
$(test)_client_sources          = Client.cpp Test.ice Derived.ice DerivedEx.ice TestI.cpp AllTests.cpp
$(test)_server_sources          = Server.cpp Test.ice Derived.ice DerivedEx.ice TestI.cpp TestIntfI.cpp
$(test)_collocated_sources      = Collocated.cpp Test.ice Derived.ice DerivedEx.ice TestI.cpp TestIntfI.cpp AllTests.cpp

tests += $(test)
