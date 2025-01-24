# Copyright (c) ZeroC, Inc.

$(project)_libraries       := $(project)_TestTransport
$(project)_cppflags        := -I$(srcdir)

$(project)_client_sources          = Client.cpp AllTests.cpp
$(project)_client_dependencies     := $(project)_TestTransport

$(project)_server_sources          = Server.cpp TestI.cpp
$(project)_server_dependencies     := $(project)_TestTransport

$(project)_TestTransport_sources = Test.ice \
                                Configuration.cpp \
                                Connector.cpp \
                                Acceptor.cpp \
                                EndpointI.cpp \
                                Transceiver.cpp \
                                EndpointFactory.cpp \
                                PluginI.cpp

tests += $(project)
