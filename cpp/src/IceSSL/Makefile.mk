#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    := IceSSL

IceSSL_targetdir        := $(libdir)
IceSSL_dependencies     := Ice
IceSSL_cppflags         := -DICESSL_API_EXPORTS

#
# Exclude sources that are not build with the default
# implementation.
#
IceSSL_excludes         += $(wildcard src/IceSSL/SChannel*.cpp)

# We exclude the following Slice files that contain only local definitions, as we don't want two header files with the
# same name.
IceSSL_excludes         += $(wildcard ../slice/IceSSL/*.ice)

ifeq ($(os),Darwin)
IceSSL_excludes         += $(wildcard src/IceSSL/OpenSSL*.cpp)
else
IceSSL_cppflags         += -DICESSL_OPENSSL_API_EXPORTS
IceSSL_excludes         += $(wildcard src/IceSSL/SecureTransport*.cpp)
endif

projects += $(project)
