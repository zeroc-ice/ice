// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class TestIntfI final : public Test::TestIntf
{
public:
    Test::ByteEnum opByte(Test::ByteEnum, Test::ByteEnum&, const Ice::Current&) final;

    Test::ShortEnum opShort(Test::ShortEnum, Test::ShortEnum&, const Ice::Current&) final;

    Test::IntEnum opInt(Test::IntEnum, Test::IntEnum&, const Ice::Current&) final;

    Test::SimpleEnum opSimple(Test::SimpleEnum, Test::SimpleEnum&, const Ice::Current&) final;

    Test::ByteEnumSeq opByteSeq(Test::ByteEnumSeq, Test::ByteEnumSeq&, const Ice::Current&) final;

    Test::ShortEnumSeq opShortSeq(Test::ShortEnumSeq, Test::ShortEnumSeq&, const Ice::Current&) final;

    Test::IntEnumSeq opIntSeq(Test::IntEnumSeq, Test::IntEnumSeq&, const Ice::Current&) final;

    Test::SimpleEnumSeq opSimpleSeq(Test::SimpleEnumSeq, Test::SimpleEnumSeq&, const Ice::Current&) final;

    void shutdown(const Ice::Current&) final;
};

#endif
