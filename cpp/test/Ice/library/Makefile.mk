# Copyright (c) ZeroC, Inc.

$(project)_libraries            := $(project)_GenCode $(project)_Consumer $(project)_AllTests

$(project)_GenCode_sources         := Test.ice

$(project)_Consumer_sources        := Consumer.cpp
$(project)_Consumer_dependencies   := $(project)_GenCode

$(project)_AllTests_sources        := AllTests.cpp
$(project)_AllTests_dependencies   := $(project)_GenCode $(project)_Consumer

$(project)_client_sources          := Client.cpp
$(project)_client_dependencies     := $(project)_AllTests

tests += $(project)
