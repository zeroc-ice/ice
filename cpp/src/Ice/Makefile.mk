# Copyright (c) ZeroC, Inc.

$(project)_libraries    = Ice

Ice_targetdir           := $(libdir)
Ice_cppflags            = -DICE_API_EXPORTS $(api_exports_cppflags) $(IceUtil_cppflags)

Ice_sliceflags          := --include-dir Ice
Ice_libs                := bz2

ifeq ($(os),Darwin)
Ice_extra_sources       := $(filter-out src/Ice/SSL/OpenSSL%.cpp src/Ice/SSL/Schannel%.cpp, $(wildcard src/Ice/SSL/*.cpp))
else
Ice_extra_sources       := $(filter-out src/Ice/SSL/SecureTransport%.cpp src/Ice/SSL/Schannel%.cpp, $(wildcard src/Ice/SSL/*.cpp))
endif

Ice_excludes            = src/Ice/DLLMain.cpp
Ice[shared]_excludes    = src/Ice/RegisterPluginsInit_min.cpp
Ice[xcodesdk]_excludes  = src/Ice/RegisterPluginsInit_min.cpp
Ice[static]_excludes    = src/Ice/RegisterPluginsInit_all.cpp

ifeq ($(os),Linux)
ifeq ($(shell pkg-config --exists libsystemd 2> /dev/null && echo yes),yes)
Ice_cppflags                            += -DICE_USE_SYSTEMD $(shell pkg-config --cflags libsystemd)
endif
endif

Ice[iphoneos]_excludes                  := $(wildcard src/Ice/CtrlCHandler.cpp $(addprefix $(currentdir)/,Tcp*.cpp Service.cpp))
Ice[iphoneos]_extra_sources             := $(wildcard $(addprefix $(currentdir)/ios/,*.cpp *.mm))
Ice[iphonesimulator]_excludes           = $(Ice[iphoneos]_excludes)
Ice[iphonesimulator]_extra_sources      = $(Ice[iphoneos]_extra_sources)

projects += $(project)
