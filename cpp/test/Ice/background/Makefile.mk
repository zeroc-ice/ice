# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

$(test)_libraries 	:= $(test)_TestTransport
$(test)_cppflags 	:= -I$(srcdir)

$(test)_client_sources = $(test-client-sources) Configuration.cpp
$(test)_server_sources = $(test-server-sources) Configuration.cpp

$(test)_TestTransport_sources = Test.ice \
				Configuration.cpp \
				Connector.cpp \
				Acceptor.cpp \
				EndpointI.cpp \
				Transceiver.cpp \
				EndpointFactory.cpp \
				PluginI.cpp

tests += $(test)
