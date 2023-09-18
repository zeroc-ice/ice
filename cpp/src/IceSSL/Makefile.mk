#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    := IceSSL

IceSSL_targetdir        := $(libdir)
IceSSL_dependencies     := Ice
IceSSL_cppflags         := -DICESSL_API_EXPORTS
IceSSL_sliceflags       := --include-dir IceSSL

#
# Exclude sources that are not build with the default
# implementation.
#
IceSSL_excludes         += $(wildcard src/IceSSL/SChannel*.cpp)

ifeq ($(os),Darwin)
IceSSL_excludes         += $(wildcard src/IceSSL/OpenSSL*.cpp)
else
IceSSL_cppflags         += -DICESSL_OPENSSL_API_EXPORTS
IceSSL_excludes         += $(wildcard src/IceSSL/SecureTransport*.cpp)
endif

projects += $(project)
