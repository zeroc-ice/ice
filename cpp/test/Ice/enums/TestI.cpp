// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

void
TestIntfI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
