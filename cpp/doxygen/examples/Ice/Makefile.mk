#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_libraries    = IceExamples

IceExamples_dependencies        := Ice
IceExamples__targetdir        := doxygen/lib

ifeq ($(os),Darwin)
IceExamples_extra_sources += $(filter-out doxygen/examples/Ice/SSL/OpenSSL%.cpp doxygen/examples/Ice/SSL/Schannel%.cpp, $(wildcard doxygen/examples/Ice/SSL/*.cpp))
else
IceExamples_extra_sources += $(filter-out doxygen/examples/Ice/SSL/SecureTransport%.cpp doxygen/examples/Ice/SSL/Schannel%.cpp, $(wildcard doxygen/examples/Ice/SSL/*.cpp))
endif

projects += $(project)
