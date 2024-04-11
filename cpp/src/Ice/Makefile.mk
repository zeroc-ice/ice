#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = Ice

Ice_targetdir           := $(libdir)
Ice_cppflags            = -DICE_API_EXPORTS $(IceUtil_cppflags)
Ice_ldflags             = $(iconv_ldflags)

Ice_sliceflags          := --include-dir Ice
Ice_libs                := bz2
Ice_extra_sources       := $(wildcard src/IceUtil/*.cpp)

ifeq ($(os),Darwin)
Ice_extra_sources       += $(filter-out src/IceSSL/OpenSSL%.cpp src/IceSSL/SChannel%.cpp, $(wildcard src/IceSSL/*.cpp))
else
Ice_extra_sources       += $(filter-out src/IceSSL/SecureTransport%.cpp src/IceSSL/SChannel%.cpp, $(wildcard src/IceSSL/*.cpp))
endif
Ice_excludes            = src/Ice/DLLMain.cpp

ifeq ($(os),Linux)
ifeq ($(shell pkg-config --exists libsystemd 2> /dev/null && echo yes),yes)
Ice_cppflags                            += -DICE_USE_SYSTEMD $(shell pkg-config --cflags libsystemd)
endif
endif

Ice[iphoneos]_excludes                  := $(wildcard src/IceUtil/CtrlCHandler.cpp $(addprefix $(currentdir)/,Tcp*.cpp Service.cpp))
Ice[iphoneos]_extra_sources             := $(wildcard $(addprefix $(currentdir)/ios/,*.cpp *.mm))
Ice[iphonesimulator]_excludes           = $(Ice[iphoneos]_excludes)
Ice[iphonesimulator]_extra_sources      = $(Ice[iphoneos]_extra_sources)

projects += $(project)
