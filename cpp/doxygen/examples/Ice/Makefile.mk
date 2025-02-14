# Copyright (c) ZeroC, Inc.

$(project)_libraries    = IceExamples
$(project)_caninstall   := no

IceExamples_dependencies     := Ice
IceExamples_targetdir        := doxygen/examples/lib

ifeq ($(os),Darwin)
IceExamples_extra_sources += $(filter-out doxygen/examples/Ice/SSL/OpenSSL%.cpp doxygen/examples/Ice/SSL/Schannel%.cpp, $(wildcard doxygen/examples/Ice/SSL/*.cpp))
IceExamples_ldflags       += -framework Security -framework CoreFoundation
else
IceExamples_extra_sources += $(filter-out doxygen/examples/Ice/SSL/SecureTransport%.cpp doxygen/examples/Ice/SSL/Schannel%.cpp, $(wildcard doxygen/examples/Ice/SSL/*.cpp))
IceExamples_ldflags       += -lssl -lcrypto
endif

projects += $(project)
