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
Ice[shared]_excludes    = src/Ice/AddDefaultPluginFactories_min.cpp
Ice[static]_excludes    = src/Ice/AddDefaultPluginFactories_all.cpp

ifeq ($(os),Linux)
ifeq ($(shell pkg-config --exists libsystemd 2> /dev/null && echo yes),yes)
Ice_cppflags                            += -DICE_USE_SYSTEMD $(shell pkg-config --cflags libsystemd)
endif
endif

ios_extrasources :=  $(wildcard $(addprefix $(currentdir)/ios/,*.cpp *.mm))
ios_excludes := $(wildcard $(addprefix $(currentdir)/,\
    CtrlCHandler.cpp \
    OutputUtil.cpp \
    Service.cpp \
    SysLoggerI.cpp \
    SystemdJournalI.cpp \
    Tcp*.cpp))

Ice[iphoneos]_excludes                  = $(ios_excludes)
Ice[iphoneos]_extra_sources             = $(ios_extrasources)

Ice[iphonesimulator]_excludes           = $(ios_excludes)
Ice[iphonesimulator]_extra_sources      = $(ios_extrasources)

projects += $(project)
