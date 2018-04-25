# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_libraries       := $(test)_TestTransport
$(test)_cppflags        := -I$(srcdir)

$(test)_client_sources          = Client.cpp AllTests.cpp
$(test)_client_dependencies     := $(test)_TestTransport

$(test)_server_sources          = Server.cpp TestI.cpp
$(test)_server_dependencies     := $(test)_TestTransport

$(test)_TestTransport_sources = Test.ice \
                                Configuration.cpp \
                                Connector.cpp \
                                Acceptor.cpp \
                                EndpointI.cpp \
                                Transceiver.cpp \
                                EndpointFactory.cpp \
                                PluginI.cpp

tests += $(test)
