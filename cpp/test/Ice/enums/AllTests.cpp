// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

TestIntfPrx
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    TestIntfPrx proxy(communicator, "test:" + helper->getTestEndpoint());

    cout << "testing enum values... " << flush;

    test(static_cast<int>(ByteEnum::benum1) == 0);
    test(static_cast<int>(ByteEnum::benum2) == 1);
    test(static_cast<int>(ByteEnum::benum3) == ByteConst1);
    test(static_cast<int>(ByteEnum::benum4) == ByteConst1 + 1);
    test(static_cast<int>(ByteEnum::benum5) == ShortConst1);
    test(static_cast<int>(ByteEnum::benum6) == ShortConst1 + 1);
    test(static_cast<int>(ByteEnum::benum7) == IntConst1);
    test(static_cast<int>(ByteEnum::benum8) == IntConst1 + 1);
    test(static_cast<int>(ByteEnum::benum9) == LongConst1);
    test(static_cast<int>(ByteEnum::benum10) == LongConst1 + 1);
    test(static_cast<int>(ByteEnum::benum11) == ByteConst2);

    test(static_cast<int>(ShortEnum::senum1) == 3);
    test(static_cast<int>(ShortEnum::senum2) == 4);
    test(static_cast<int>(ShortEnum::senum3) == ByteConst1);
    test(static_cast<int>(ShortEnum::senum4) == ByteConst1 + 1);
    test(static_cast<int>(ShortEnum::senum5) == ShortConst1);
    test(static_cast<int>(ShortEnum::senum6) == ShortConst1 + 1);
    test(static_cast<int>(ShortEnum::senum7) == IntConst1);
    test(static_cast<int>(ShortEnum::senum8) == IntConst1 + 1);
    test(static_cast<int>(ShortEnum::senum9) == LongConst1);
    test(static_cast<int>(ShortEnum::senum10) == LongConst1 + 1);
    test(static_cast<int>(ShortEnum::senum11) == ShortConst2);

    test(static_cast<int>(IntEnum::ienum1) == 0);
    test(static_cast<int>(IntEnum::ienum2) == 1);
    test(static_cast<int>(IntEnum::ienum3) == ByteConst1);
    test(static_cast<int>(IntEnum::ienum4) == ByteConst1 + 1);
    test(static_cast<int>(IntEnum::ienum5) == ShortConst1);
    test(static_cast<int>(IntEnum::ienum6) == ShortConst1 + 1);
    test(static_cast<int>(IntEnum::ienum7) == IntConst1);
    test(static_cast<int>(IntEnum::ienum8) == IntConst1 + 1);
    test(static_cast<int>(IntEnum::ienum9) == LongConst1);
    test(static_cast<int>(IntEnum::ienum10) == LongConst1 + 1);
    test(static_cast<int>(IntEnum::ienum11) == IntConst2);
    test(static_cast<int>(IntEnum::ienum12) == LongConst2);

    test(static_cast<int>(SimpleEnum::red) == 0);
    test(static_cast<int>(SimpleEnum::green) == 1);
    test(static_cast<int>(SimpleEnum::blue) == 2);

    cout << "ok" << endl;

    cout << "testing enum operations... " << flush;

    ByteEnum byteEnum;
    test(proxy->opByte(ByteEnum::benum1, byteEnum) == ByteEnum::benum1);
    test(byteEnum == ByteEnum::benum1);
    test(proxy->opByte(ByteEnum::benum11, byteEnum) == ByteEnum::benum11);
    test(byteEnum == ByteEnum::benum11);

    ShortEnum shortEnum;
    test(proxy->opShort(ShortEnum::senum1, shortEnum) == ShortEnum::senum1);
    test(shortEnum == ShortEnum::senum1);
    test(proxy->opShort(ShortEnum::senum11, shortEnum) == ShortEnum::senum11);
    test(shortEnum == ShortEnum::senum11);

    IntEnum intEnum;
    test(proxy->opInt(IntEnum::ienum1, intEnum) == IntEnum::ienum1);
    test(intEnum == IntEnum::ienum1);
    test(proxy->opInt(IntEnum::ienum11, intEnum) == IntEnum::ienum11);
    test(intEnum == IntEnum::ienum11);
    test(proxy->opInt(IntEnum::ienum12, intEnum) == IntEnum::ienum12);
    test(intEnum == IntEnum::ienum12);

    SimpleEnum s;
    test(proxy->opSimple(SimpleEnum::green, s) == SimpleEnum::green);
    test(s == SimpleEnum::green);

    cout << "ok" << endl;

    cout << "testing enum sequences operations... " << flush;

    {
        ByteEnum values[] = {
            ByteEnum::benum1,
            ByteEnum::benum2,
            ByteEnum::benum3,
            ByteEnum::benum4,
            ByteEnum::benum5,
            ByteEnum::benum6,
            ByteEnum::benum7,
            ByteEnum::benum8,
            ByteEnum::benum9,
            ByteEnum::benum10,
            ByteEnum::benum11};
        ByteEnumSeq b1(&values[0], &values[0] + sizeof(values) / sizeof(ByteEnum));

        ByteEnumSeq b2;
        ByteEnumSeq b3 = proxy->opByteSeq(b1, b2);

        for (size_t i = 0; i < b1.size(); ++i)
        {
            test(b1[i] == b2[i]);
            test(b1[i] == b3[i]);
        }
    }

    {
        ShortEnum values[] = {
            ShortEnum::senum1,
            ShortEnum::senum2,
            ShortEnum::senum3,
            ShortEnum::senum4,
            ShortEnum::senum5,
            ShortEnum::senum6,
            ShortEnum::senum7,
            ShortEnum::senum8,
            ShortEnum::senum9,
            ShortEnum::senum10,
            ShortEnum::senum11};
        ShortEnumSeq s1(&values[0], &values[0] + sizeof(values) / sizeof(ShortEnum));

        ShortEnumSeq s2;
        ShortEnumSeq s3 = proxy->opShortSeq(s1, s2);

        for (size_t i = 0; i < s1.size(); ++i)
        {
            test(s1[i] == s2[i]);
            test(s1[i] == s3[i]);
        }
    }

    {
        IntEnum values[] = {
            IntEnum::ienum1,
            IntEnum::ienum2,
            IntEnum::ienum3,
            IntEnum::ienum4,
            IntEnum::ienum5,
            IntEnum::ienum6,
            IntEnum::ienum7,
            IntEnum::ienum8,
            IntEnum::ienum9,
            IntEnum::ienum10,
            IntEnum::ienum11};
        IntEnumSeq i1(&values[0], &values[0] + sizeof(values) / sizeof(IntEnum));

        IntEnumSeq i2;
        IntEnumSeq i3 = proxy->opIntSeq(i1, i2);

        for (size_t i = 0; i < i1.size(); ++i)
        {
            test(i1[i] == i2[i]);
            test(i1[i] == i3[i]);
        }
    }

    {
        SimpleEnum values[] = {SimpleEnum::red, SimpleEnum::green, SimpleEnum::blue};
        SimpleEnumSeq s1(&values[0], &values[0] + sizeof(values) / sizeof(SimpleEnum));

        SimpleEnumSeq s2;
        SimpleEnumSeq s3 = proxy->opSimpleSeq(s1, s2);

        for (size_t i = 0; i < s1.size(); ++i)
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
    catch (const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opByte(static_cast<ByteEnum>(127), byteEnum); // Invalid enumerator
        test(false);
    }
    catch (const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opShort(static_cast<ShortEnum>(-1), shortEnum); // Negative enumerators are not supported
        test(false);
    }
    catch (const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opShort(static_cast<ShortEnum>(0), shortEnum); // Invalid enumerator
        test(false);
    }
    catch (const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opShort(static_cast<ShortEnum>(32767), shortEnum); // Invalid enumerator
        test(false);
    }
    catch (const Ice::MarshalException&)
    {
    }

    try
    {
        proxy->opInt(static_cast<IntEnum>(-1), intEnum); // Negative enumerators are not supported
        test(false);
    }
    catch (const Ice::MarshalException&)
    {
    }

    cout << "ok" << endl;

    return proxy;
}
