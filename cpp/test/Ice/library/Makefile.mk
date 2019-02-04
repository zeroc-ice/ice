#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_libraries            := $(test)_GenCode $(test)_Consumer $(test)_AllTests

$(test)_GenCode_sources         := Test.ice

$(test)_Consumer_sources        := Consumer.cpp
$(test)_Consumer_dependencies   := $(test)_GenCode

$(test)_AllTests_sources        := AllTests.cpp
$(test)_AllTests_dependencies   := $(test)_GenCode $(test)_Consumer

$(test)_client_sources          := Client.cpp
$(test)_client_dependencies     := $(test)_AllTests

tests += $(test)
