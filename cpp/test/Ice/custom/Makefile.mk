# Copyright (c) ZeroC, Inc.

$(project)_client_sources          = Client.cpp \
                                  AllTests.cpp \
                                  Test.ice \
                                  Wstring.ice \
                                  MyByteSeq.cpp \
                                  StringConverterI.cpp

$(project)_server_sources          = Server.cpp \
                                  TestI.cpp \
                                  WstringI.cpp \
                                  Test.ice \
                                  Wstring.ice \
                                  MyByteSeq.cpp \
                                  StringConverterI.cpp

$(project)_serveramd_sources       = ServerAMD.cpp \
                                  TestAMDI.cpp \
                                  WstringAMDI.cpp \
                                  TestAMD.ice \
                                  WstringAMD.ice \
                                  MyByteSeq.cpp \
                                  StringConverterI.cpp

# Extra -I because we include <MyByteSeq.h> in the generated code.
# ICE_UNALIGNED means enable unaligned decoding for integral types such as int32_t. It requires a little endian CPU.
# TODO: should be turn on ICE_UNALIGNED only on some platforms?
$(project)_cppflags        := -I$(project) -DICE_UNALIGNED

tests += $(project)
