// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestIntfI : virtual public Test::TestIntf
{
public:

    virtual Test::ByteEnum opByte(Test::ByteEnum, Test::ByteEnum&, const Ice::Current&);

    virtual Test::ShortEnum opShort(Test::ShortEnum, Test::ShortEnum&, const Ice::Current&);

    virtual Test::IntEnum opInt(Test::IntEnum, Test::IntEnum&, const Ice::Current&);

    virtual Test::SimpleEnum opSimple(Test::SimpleEnum, Test::SimpleEnum&, const Ice::Current&);

    virtual Test::ByteEnumSeq opByteSeq(const Test::ByteEnumSeq&, Test::ByteEnumSeq&, const Ice::Current&);

    virtual Test::ShortEnumSeq opShortSeq(const Test::ShortEnumSeq&, Test::ShortEnumSeq&, const ::Ice::Current&);

    virtual Test::IntEnumSeq opIntSeq(const Test::IntEnumSeq&, Test::IntEnumSeq&, const ::Ice::Current&);

    virtual Test::SimpleEnumSeq opSimpleSeq(const Test::SimpleEnumSeq&, Test::SimpleEnumSeq&, const ::Ice::Current&);

    virtual void shutdown(const Ice::Current&);
};

#endif
