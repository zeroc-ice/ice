// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

Test::ByteEnum
TestIntfI::opByte(Test::ByteEnum b1, Test::ByteEnum& b2, const Ice::Current&)
{
    b2 = b1;
    return b1;
}

Test::ShortEnum
TestIntfI::opShort(Test::ShortEnum s1, Test::ShortEnum& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::IntEnum
TestIntfI::opInt(Test::IntEnum i1, Test::IntEnum& i2, const Ice::Current&)
{
    i2 = i1;
    return i1;
}

Test::SimpleEnum
TestIntfI::opSimple(Test::SimpleEnum s1, Test::SimpleEnum& s2, const Ice::Current&)
{
    s2 = s1;
    return s1;
}

Test::ByteEnumSeq
TestIntfI::opByteSeq(ICE_IN(Test::ByteEnumSeq) bs1, Test::ByteEnumSeq& bs2, const Ice::Current&)
{
    bs2 = bs1;
    return bs1;
}

Test::ShortEnumSeq
TestIntfI::opShortSeq(ICE_IN(Test::ShortEnumSeq) ss1, Test::ShortEnumSeq& ss2, const ::Ice::Current&)
{
    ss2 = ss1;
    return ss1;
}

Test::IntEnumSeq
TestIntfI::opIntSeq(ICE_IN(Test::IntEnumSeq) is1, Test::IntEnumSeq& is2, const ::Ice::Current&)
{
    is2 = is1;
    return is1;
}

Test::SimpleEnumSeq
TestIntfI::opSimpleSeq(ICE_IN(Test::SimpleEnumSeq) ss1, Test::SimpleEnumSeq& ss2, const ::Ice::Current&)
{
    ss2 = ss1;
    return ss1;
}

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
