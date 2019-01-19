#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(test)_client_sources          = Client.cpp \
                                  AllTests.cpp \
                                  Test.ice \
                                  Wstring.ice \
                                  MyByteSeq.cpp \
                                  StringConverterI.cpp

$(test)_server_sources          = Server.cpp \
                                  TestI.cpp \
                                  WstringI.cpp \
                                  Test.ice \
                                  Wstring.ice \
                                  MyByteSeq.cpp \
                                  StringConverterI.cpp

$(test)_serveramd_sources       = ServerAMD.cpp \
                                  TestAMDI.cpp \
                                  WstringAMDI.cpp \
                                  TestAMD.ice \
                                  WstringAMD.ice \
                                  MyByteSeq.cpp \
                                  StringConverterI.cpp

#
# Disable var tracking assignments for Linux with this test
#
ifneq ($(linux_id),)
    $(test)_cppflags += $(if $(filter yes,$(OPTIMIZE)),-fno-var-tracking-assignments)
endif

tests += $(test)
