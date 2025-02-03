# Copyright (c) ZeroC, Inc.

$(project)_client_sources  = Test.ice \
                          Client.cpp \
                          AllTests.cpp \
                          Twoways.cpp \
                          Oneways.cpp \
                          TwowaysAMI.cpp \
                          OnewaysAMI.cpp \
                          BatchOneways.cpp \
                          BatchOnewaysAMI.cpp

tests += $(project)
