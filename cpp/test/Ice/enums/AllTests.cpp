// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

TestIntfPrxPtr
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = "test:" + getTestEndpoint(communicator, 0);
    Ice::ObjectPrxPtr obj = communicator->stringToProxy(ref);
    test(obj);
    TestIntfPrxPtr proxy = ICE_CHECKED_CAST(TestIntfPrx, obj);
    test(proxy);

    cout << "testing enum values... " << flush;

    test(static_cast<int>(ICE_ENUM(ByteEnum, benum1)) == 0);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum2)) == 1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum3)) == ByteConst1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum4)) == ByteConst1 + 1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum5)) == ShortConst1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum6)) == ShortConst1 + 1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum7)) == IntConst1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum8)) == IntConst1 + 1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum9)) == LongConst1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum10)) == LongConst1 + 1);
    test(static_cast<int>(ICE_ENUM(ByteEnum, benum11)) == ByteConst2);

    test(static_cast<int>(ICE_ENUM(ShortEnum, senum1)) == 3);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum2)) == 4);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum3)) == ByteConst1);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum4)) == ByteConst1 + 1);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum5)) == ShortConst1);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum6)) == ShortConst1 + 1);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum7)) == IntConst1);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum8)) == IntConst1 + 1);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum9)) == LongConst1);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum10)) == LongConst1 + 1);
    test(static_cast<int>(ICE_ENUM(ShortEnum, senum11)) == ShortConst2);

    test(static_cast<int>(ICE_ENUM(IntEnum, ienum1)) == 0);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum2)) == 1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum3)) == ByteConst1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum4)) == ByteConst1 + 1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum5)) == ShortConst1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum6)) == ShortConst1 + 1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum7)) == IntConst1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum8)) == IntConst1 + 1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum9)) == LongConst1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum10)) == LongConst1 + 1);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum11)) == IntConst2);
    test(static_cast<int>(ICE_ENUM(IntEnum, ienum12)) == LongConst2);

    test(static_cast<int>(ICE_ENUM(SimpleEnum, red)) == 0);
    test(static_cast<int>(ICE_ENUM(SimpleEnum, green)) == 1);
    test(static_cast<int>(ICE_ENUM(SimpleEnum, blue)) == 2);

    cout << "ok" << endl;

#ifndef ICE_CPP11_MAPPING
    cout << "testing enum streaming... " << flush;

    Ice::ByteSeq bytes;

    const bool encoding_1_0 = communicator->getProperties()->getProperty("Ice.Default.EncodingVersion") == "1.0";

    {
        Ice::OutputStream out(communicator);
        out.write(ICE_ENUM(ByteEnum, benum11));
        out.finished(bytes);
        test(bytes.size() == 1); // ByteEnum should require one byte
    }

    {
        Ice::OutputStream out(communicator);
        out.write(ICE_ENUM(ShortEnum, senum11));
        out.finished(bytes);
        test(bytes.size() == (encoding_1_0 ? 2 : 5));
    }

    {
        Ice::OutputStream out(communicator);
        out.write(ICE_ENUM(IntEnum, ienum11));
        out.finished(bytes);
        test(bytes.size() == (encoding_1_0 ? 4 : 5));
    }

    {
        Ice::OutputStream out(communicator);
        out.write(ICE_ENUM(SimpleEnum, blue));
        out.finished(bytes);
        test(bytes.size() == 1); // SimpleEnum should require one byte
    }

    cout << "ok" << endl;
#endif

    cout << "testing enum operations... " << flush;

    ByteEnum byteEnum;
    test(proxy->opByte(ICE_ENUM(ByteEnum, benum1), byteEnum) == ICE_ENUM(ByteEnum, benum1));
    test(byteEnum == ICE_ENUM(ByteEnum, benum1));
    test(proxy->opByte(ICE_ENUM(ByteEnum, benum11), byteEnum) == ICE_ENUM(ByteEnum, benum11));
    test(byteEnum == ICE_ENUM(ByteEnum, benum11));

    ShortEnum shortEnum;
    test(proxy->opShort(ICE_ENUM(ShortEnum, senum1), shortEnum) == ICE_ENUM(ShortEnum, senum1));
    test(shortEnum == ICE_ENUM(ShortEnum, senum1));
    test(proxy->opShort(ICE_ENUM(ShortEnum, senum11), shortEnum) == ICE_ENUM(ShortEnum, senum11));
    test(shortEnum == ICE_ENUM(ShortEnum, senum11));

    IntEnum intEnum;
    test(proxy->opInt(ICE_ENUM(IntEnum, ienum1), intEnum) == ICE_ENUM(IntEnum, ienum1));
    test(intEnum == ICE_ENUM(IntEnum, ienum1));
    test(proxy->opInt(ICE_ENUM(IntEnum, ienum11), intEnum) == ICE_ENUM(IntEnum, ienum11));
    test(intEnum == ICE_ENUM(IntEnum, ienum11));
    test(proxy->opInt(ICE_ENUM(IntEnum, ienum12), intEnum) == ICE_ENUM(IntEnum, ienum12));
    test(intEnum == ICE_ENUM(IntEnum, ienum12));

    SimpleEnum s;
    test(proxy->opSimple(ICE_ENUM(SimpleEnum, green), s) == ICE_ENUM(SimpleEnum, green));
    test(s == ICE_ENUM(SimpleEnum, green));

    cout << "ok" << endl;

    cout << "testing enum sequences operations... " << flush;

    {
        ByteEnum values[] =
        {
            ICE_ENUM(ByteEnum, benum1),
            ICE_ENUM(ByteEnum, benum2),
            ICE_ENUM(ByteEnum, benum3),
            ICE_ENUM(ByteEnum, benum4),
            ICE_ENUM(ByteEnum, benum5),
            ICE_ENUM(ByteEnum, benum6),
            ICE_ENUM(ByteEnum, benum7),
            ICE_ENUM(ByteEnum, benum8),
            ICE_ENUM(ByteEnum, benum9),
            ICE_ENUM(ByteEnum, benum10),
            ICE_ENUM(ByteEnum, benum11)
        };
        ByteEnumSeq b1(&values[0], &values[0] + sizeof(values) / sizeof(ByteEnum));

        ByteEnumSeq b2;
        ByteEnumSeq b3 = proxy->opByteSeq(b1, b2);

        for(size_t i = 0; i < b1.size(); ++i)
        {
            test(b1[i] == b2[i]);
            test(b1[i] == b3[i]);
        }
    }

    {
        ShortEnum values[] = 
        {
            ICE_ENUM(ShortEnum, senum1),
            ICE_ENUM(ShortEnum, senum2),
            ICE_ENUM(ShortEnum, senum3),
            ICE_ENUM(ShortEnum, senum4),
            ICE_ENUM(ShortEnum, senum5),
            ICE_ENUM(ShortEnum, senum6),
            ICE_ENUM(ShortEnum, senum7),
            ICE_ENUM(ShortEnum, senum8),
            ICE_ENUM(ShortEnum, senum9),
            ICE_ENUM(ShortEnum, senum10),
            ICE_ENUM(ShortEnum, senum11)
        };
        ShortEnumSeq s1(&values[0], &values[0] + sizeof(values) / sizeof(ShortEnum));

        ShortEnumSeq s2;
        ShortEnumSeq s3 = proxy->opShortSeq(s1, s2);

        for(size_t i = 0; i < s1.size(); ++i)
        {
            test(s1[i] == s2[i]);
            test(s1[i] == s3[i]);
        }
    }

    {
        IntEnum values[] =
        {
            ICE_ENUM(IntEnum, ienum1),
            ICE_ENUM(IntEnum, ienum2),
            ICE_ENUM(IntEnum, ienum3),
            ICE_ENUM(IntEnum, ienum4),
            ICE_ENUM(IntEnum, ienum5),
            ICE_ENUM(IntEnum, ienum6),
            ICE_ENUM(IntEnum, ienum7),
            ICE_ENUM(IntEnum, ienum8),
            ICE_ENUM(IntEnum, ienum9),
            ICE_ENUM(IntEnum, ienum10),
            ICE_ENUM(IntEnum, ienum11)
        };
        IntEnumSeq i1(&values[0], &values[0] + sizeof(values) / sizeof(IntEnum));

        IntEnumSeq i2;
        IntEnumSeq i3 = proxy->opIntSeq(i1, i2);

        for(size_t i = 0; i < i1.size(); ++i)
        {
            test(i1[i] == i2[i]);
            test(i1[i] == i3[i]);
        }
    }

    {
        SimpleEnum values[] =
        {
            ICE_ENUM(SimpleEnum, red),
            ICE_ENUM(SimpleEnum, green),
            ICE_ENUM(SimpleEnum, blue)
        };
        SimpleEnumSeq s1(&values[0], &values[0] + sizeof(values) / sizeof(SimpleEnum));

        SimpleEnumSeq s2;
        SimpleEnumSeq s3 = proxy->opSimpleSeq(s1, s2);

        for(size_t i = 0; i < s1.size(); ++i)
        {
            test(s1[i] == s2[i]);
            test(s1[i] == s3[i]);
        }
    }

    cout << "ok" << endl;

    cout << "testing enum exceptions... " << flush;

    try
    {
        proxy->opByte(static_cast<ByteEnum>(-1), byteEnum); // Negative enumerators are not supported
        test(false);
    }
    catch(const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opByte(static_cast<ByteEnum>(127), byteEnum); // Invalid enumerator
        test(false);
    }
    catch(const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opShort(static_cast<ShortEnum>(-1), shortEnum); // Negative enumerators are not supported
        test(false);
    }
    catch(const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opShort(static_cast<ShortEnum>(0), shortEnum); // Invalid enumerator
        test(false);
    }
    catch(const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opShort(static_cast<ShortEnum>(32767), shortEnum); // Invalid enumerator
        test(false);
    }
    catch(const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opInt(static_cast<IntEnum>(-1), intEnum); // Negative enumerators are not supported
        test(false);
    }
    catch(const Ice::MarshalException&)
    {
    }

    cout << "ok" << endl;

    return proxy;
}
