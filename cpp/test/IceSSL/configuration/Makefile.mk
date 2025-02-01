# Copyright (c) ZeroC, Inc.

$(project)_dependencies = TestCommon Ice

$(project)_client_sources := Client.cpp AllTests.cpp Test.ice TestI.cpp

ifeq ($(os),Darwin)
    $(project)_client_sources += SecureTransportTests.cpp
    $(project)_client_ldflags = -framework Security -framework CoreFoundation
endif

ifeq ($(os),Linux)
    $(project)_client_sources += OpenSSLTests.cpp
    $(project)_client_ldflags = -lssl -lcrypto
endif

tests += $(project)
