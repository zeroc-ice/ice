// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace Ice;
using namespace Test;

ByteEnum
TestIntfI::opByte(ByteEnum b1, ByteEnum& b2, const Current&)
{
    b2 = b1;
    return b1;
}

ShortEnum
TestIntfI::opShort(ShortEnum s1, ShortEnum& s2, const Current&)
{
    s2 = s1;
    return s1;
}

IntEnum
TestIntfI::opInt(IntEnum i1, IntEnum& i2, const Current&)
{
    i2 = i1;
    return i1;
}

SimpleEnum
TestIntfI::opSimple(SimpleEnum s1, SimpleEnum& s2, const Current&)
{
    s2 = s1;
    return s1;
}

ByteEnumSeq
TestIntfI::opByteSeq(ByteEnumSeq bs1, ByteEnumSeq& bs2, const Current&)
{
    bs2 = bs1;
    return bs1;
}

ShortEnumSeq
TestIntfI::opShortSeq(ShortEnumSeq ss1, ShortEnumSeq& ss2, const ::Current&)
{
    ss2 = ss1;
    return ss1;
}

IntEnumSeq
TestIntfI::opIntSeq(IntEnumSeq is1, IntEnumSeq& is2, const ::Current&)
{
    is2 = is1;
    return is1;
}

SimpleEnumSeq
TestIntfI::opSimpleSeq(SimpleEnumSeq ss1, SimpleEnumSeq& ss2, const ::Current&)
{
    ss2 = ss1;
    return ss1;
}

void
TestIntfI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
