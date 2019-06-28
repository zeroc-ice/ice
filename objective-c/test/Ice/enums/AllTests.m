//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <TestCommon.h>
#import <EnumTest.h>

TestEnumTestIntfPrx*
enumAllTests(id<ICECommunicator> communicator)
{

    ICEObjectPrx* obj = [communicator stringToProxy:@"test:default -p 12010"];
    test(obj);
    TestEnumTestIntfPrx* proxy = [TestEnumTestIntfPrx checkedCast:obj];
    test(proxy);

    tprintf("testing enum values... ");

    test((int)(TestEnumbenum1) == 0);
    test((int)(TestEnumbenum2) == 1);
    test((int)(TestEnumbenum3) == TestEnumByteConst1);
    test((int)(TestEnumbenum4) == TestEnumByteConst1 + 1);
    test((int)(TestEnumbenum5) == TestEnumShortConst1);
    test((int)(TestEnumbenum6) == TestEnumShortConst1 + 1);
    test((int)(TestEnumbenum7) == TestEnumIntConst1);
    test((int)(TestEnumbenum8) == TestEnumIntConst1 + 1);
    test((int)(TestEnumbenum9) == TestEnumLongConst1);
    test((int)(TestEnumbenum10) == TestEnumLongConst1 + 1);
    test((int)(TestEnumbenum11) == TestEnumByteConst2);

    test((int)(TestEnumsenum1) == 3);
    test((int)(TestEnumsenum2) == 4);
    test((int)(TestEnumsenum3) == TestEnumByteConst1);
    test((int)(TestEnumsenum4) == TestEnumByteConst1 + 1);
    test((int)(TestEnumsenum5) == TestEnumShortConst1);
    test((int)(TestEnumsenum6) == TestEnumShortConst1 + 1);
    test((int)(TestEnumsenum7) == TestEnumIntConst1);
    test((int)(TestEnumsenum8) == TestEnumIntConst1 + 1);
    test((int)(TestEnumsenum9) == TestEnumLongConst1);
    test((int)(TestEnumsenum10) == TestEnumLongConst1 + 1);
    test((int)(TestEnumsenum11) == TestEnumShortConst2);

    test((int)(TestEnumienum1) == 0);
    test((int)(TestEnumienum2) == 1);
    test((int)(TestEnumienum3) == TestEnumByteConst1);
    test((int)(TestEnumienum4) == TestEnumByteConst1 + 1);
    test((int)(TestEnumienum5) == TestEnumShortConst1);
    test((int)(TestEnumienum6) == TestEnumShortConst1 + 1);
    test((int)(TestEnumienum7) == TestEnumIntConst1);
    test((int)(TestEnumienum8) == TestEnumIntConst1 + 1);
    test((int)(TestEnumienum9) == TestEnumLongConst1);
    test((int)(TestEnumienum10) == TestEnumLongConst1 + 1);
    test((int)(TestEnumienum11) == TestEnumIntConst2);
    test((int)(TestEnumienum12) == TestEnumLongConst2);

    test((int)(TestEnumred) == 0);
    test((int)(TestEnumgreen) == 1);
    test((int)(TestEnumblue) == 2);

    tprintf("ok\n");

    tprintf("testing enum operations... ");

    TestEnumByteEnum byteEnum;
    test([proxy opByte:TestEnumbenum1 b2:&byteEnum] == TestEnumbenum1);
    test(byteEnum == TestEnumbenum1);
    test([proxy opByte:TestEnumbenum11 b2:&byteEnum] == TestEnumbenum11);
    test(byteEnum == TestEnumbenum11);

    TestEnumShortEnum shortEnum;
    test([proxy opShort:TestEnumsenum1 s2:&shortEnum] == TestEnumsenum1);
    test(shortEnum == TestEnumsenum1);
    test([proxy opShort:TestEnumsenum11 s2:&shortEnum] == TestEnumsenum11);
    test(shortEnum == TestEnumsenum11);

    TestEnumIntEnum intEnum;
    test([proxy opInt:TestEnumienum1 i2:&intEnum] == TestEnumienum1);
    test(intEnum == TestEnumienum1);
    test([proxy opInt:TestEnumienum11 i2:&intEnum] == TestEnumienum11);
    test(intEnum == TestEnumienum11);
    test([proxy opInt:TestEnumienum12 i2:&intEnum] == TestEnumienum12);
    test(intEnum == TestEnumienum12);

    TestEnumSimpleEnum s;
    test([proxy opSimple:TestEnumgreen s2:&s] == TestEnumgreen);
    test(s == TestEnumgreen);
    tprintf("ok\n");

    tprintf("testing enum sequences operations... ");

    {
        TestEnumByteEnum values[] = {TestEnumbenum1, TestEnumbenum2, TestEnumbenum3, TestEnumbenum4, TestEnumbenum5,
                                     TestEnumbenum6, TestEnumbenum7, TestEnumbenum8, TestEnumbenum9, TestEnumbenum10,
                                     TestEnumbenum11};

        NSUInteger enumSize = sizeof(TestEnumByteEnum);
        NSUInteger length = sizeof(values);
        NSUInteger elements = length / enumSize;

        TestEnumMutableByteEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableByteEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        TestEnumByteEnumSeq* enumSeq3 = [proxy opByteSeq:enumSeq1 b2:&enumSeq2];

        ICEByte* p1 = (ICEByte *)[enumSeq1 bytes];
        ICEByte* p2 = (ICEByte *)[enumSeq2 bytes];
        ICEByte* p3 = (ICEByte *)[enumSeq3 bytes];

        for(NSUInteger i = 0; i < elements; ++i)
        {
            test(*p1 == *p2);
            test(*p1 == *p3);
            p1++;
            p2++;
            p3++;
        }
    }

    {
        TestEnumShortEnum values[] = {TestEnumsenum1, TestEnumsenum2, TestEnumsenum3, TestEnumsenum4, TestEnumsenum5,
                                     TestEnumsenum6, TestEnumsenum7, TestEnumsenum8, TestEnumsenum9, TestEnumsenum10,
                                     TestEnumsenum11};

        NSUInteger enumSize = sizeof(TestEnumShortEnum);
        NSUInteger length = sizeof(values);
        NSUInteger elements = length / enumSize;

        TestEnumMutableShortEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableShortEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        TestEnumShortEnumSeq* enumSeq3 = [proxy opShortSeq:enumSeq1 s2:&enumSeq2];

        ICEByte* p1 = (ICEByte *)[enumSeq1 bytes];
        ICEByte* p2 = (ICEByte *)[enumSeq2 bytes];
        ICEByte* p3 = (ICEByte *)[enumSeq3 bytes];

        for(NSUInteger i = 0; i < elements; ++i)
        {
            test(*p1 == *p2);
            test(*p1 == *p3);
            p1++;
            p2++;
            p3++;
        }
    }

    {

        TestEnumIntEnum values[] = {TestEnumienum1, TestEnumienum2, TestEnumienum3, TestEnumienum4, TestEnumienum5,
                                     TestEnumienum6, TestEnumienum7, TestEnumienum8, TestEnumienum9, TestEnumienum10,
                                     TestEnumienum11};

        NSUInteger enumSize = sizeof(TestEnumShortEnum);
        NSUInteger length = sizeof(values);
        NSUInteger elements = length / enumSize;

        TestEnumMutableShortEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableShortEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        TestEnumShortEnumSeq* enumSeq3 = [proxy opIntSeq:enumSeq1 i2:&enumSeq2];

        ICEByte* p1 = (ICEByte *)[enumSeq1 bytes];
        ICEByte* p2 = (ICEByte *)[enumSeq2 bytes];
        ICEByte* p3 = (ICEByte *)[enumSeq3 bytes];

        for(NSUInteger i = 0; i < elements; ++i)
        {
            test(*p1 == *p2);
            test(*p1 == *p3);
            p1++;
            p2++;
            p3++;
        }
    }

    {

        TestEnumSimpleEnum values[] = {TestEnumred, TestEnumgreen, TestEnumblue};

        NSUInteger enumSize = sizeof(TestEnumShortEnum);
        NSUInteger length = sizeof(values);
        NSUInteger elements = length / enumSize;

        TestEnumMutableShortEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableShortEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        TestEnumShortEnumSeq* enumSeq3 = [proxy opSimpleSeq:enumSeq1 s2:&enumSeq2];

        ICEByte* p1 = (ICEByte *)[enumSeq1 bytes];
        ICEByte* p2 = (ICEByte *)[enumSeq2 bytes];
        ICEByte* p3 = (ICEByte *)[enumSeq3 bytes];

        for(NSUInteger i = 0; i < elements; ++i)
        {
            test(*p1 == *p2);
            test(*p1 == *p3);
            p1++;
            p2++;
            p3++;
        }
    }

    tprintf("ok\n");

    tprintf("testing enum exceptions... ");

    @try
    {
        [proxy opByte:(TestEnumByteEnum)-1 b2:&byteEnum]; // Negative enumerators are not supported
        test(NO);
    }
    @catch(ICEMarshalException*)
    {
    }

    @try
    {
        [proxy opByte:(TestEnumByteEnum)127 b2:&byteEnum]; // Invalid enumerator
        test(NO);
    }
    @catch(ICEMarshalException*)
    {
    }

    @try
    {
        [proxy opShort:(TestEnumShortEnum)-1 s2:&shortEnum]; // Negative enumerators are not supported
        test(NO);
    }
    @catch(const ICEMarshalException*)
    {
    }

    @try
    {
        [proxy opShort:(TestEnumShortEnum)0 s2:&shortEnum]; // Invalid enumerator
        test(NO);
    }
    @catch(ICEMarshalException*)
    {
    }

    @try
    {
        [proxy opShort:(TestEnumShortEnum)32767 s2:&shortEnum]; // Invalid enumerator
        test(NO);
    }
    @catch(ICEMarshalException*)
    {
    }

    @try
    {
        [proxy opInt:(TestEnumIntEnum)-1 i2:&intEnum]; // Negative enumerators are not supported
        test(NO);
    }
    @catch(ICEMarshalException*)
    {
    }

    {
        TestEnumByteEnum values[] = {TestEnumbenum1, TestEnumbenum2, TestEnumbenum3, TestEnumbenum4, TestEnumbenum5,
                                     TestEnumbenum6, (TestEnumByteEnum)-1, TestEnumbenum8, TestEnumbenum9, TestEnumbenum10,
                                     TestEnumbenum11}; // Negative enumerators are not supported

        NSUInteger length = sizeof(values);

        TestEnumMutableByteEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableByteEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        @try
        {
            [proxy opByteSeq:enumSeq1 b2:&enumSeq2];
            test(NO);
        }
        @catch(ICEMarshalException*)
        {
        }
    }

    {
        TestEnumByteEnum values[] = {TestEnumbenum1, TestEnumbenum2, TestEnumbenum3, TestEnumbenum4, TestEnumbenum5,
                                     TestEnumbenum6, (TestEnumByteEnum)127, TestEnumbenum8, TestEnumbenum9, TestEnumbenum10,
                                     TestEnumbenum11}; // Invalid enumerator

        NSUInteger length = sizeof(values);

        TestEnumMutableByteEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableByteEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        @try
        {
            [proxy opByteSeq:enumSeq1 b2:&enumSeq2];
            test(NO);
        }
        @catch(ICEMarshalException*)
        {
        }
    }

    {

        TestEnumShortEnum values[] = {TestEnumsenum1, TestEnumsenum2, TestEnumsenum3, TestEnumsenum4, TestEnumsenum5,
                                     (TestEnumShortEnum)-1, TestEnumsenum7, TestEnumsenum8, TestEnumsenum9, TestEnumsenum10,
                                     TestEnumsenum11}; // Negative enumerators are not supported

        NSUInteger length = sizeof(values);

        TestEnumMutableShortEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableShortEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        @try
        {
            [proxy opShortSeq:enumSeq1 s2:&enumSeq2];
            test(NO);
        }
        @catch(ICEMarshalException*)
        {
        }
    }

    {

        TestEnumShortEnum values[] = {TestEnumsenum1, TestEnumsenum2, TestEnumsenum3, TestEnumsenum4, TestEnumsenum5,
                                     (TestEnumShortEnum)0, TestEnumsenum7, TestEnumsenum8, TestEnumsenum9, TestEnumsenum10,
                                     TestEnumsenum11}; // Invalid enumerator

        NSUInteger length = sizeof(values);

        TestEnumMutableShortEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableShortEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        @try
        {
            [proxy opShortSeq:enumSeq1 s2:&enumSeq2];
            test(NO);
        }
        @catch(ICEMarshalException*)
        {
        }
    }

    {

        TestEnumShortEnum values[] = {TestEnumsenum1, TestEnumsenum2, TestEnumsenum3, TestEnumsenum4, TestEnumsenum5,
                                     (TestEnumShortEnum)32767, TestEnumsenum7, TestEnumsenum8, TestEnumsenum9, TestEnumsenum10,
                                     TestEnumsenum11}; // Invalid enumerator

        NSUInteger length = sizeof(values);

        TestEnumMutableShortEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableShortEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        @try
        {
            [proxy opShortSeq:enumSeq1 s2:&enumSeq2];
            test(NO);
        }
        @catch(ICEMarshalException*)
        {
        }
    }

    {

        TestEnumIntEnum values[] = {TestEnumienum1, TestEnumienum2, TestEnumienum3, TestEnumienum4, TestEnumienum5,
                                     (TestEnumIntEnum)-1, TestEnumienum7, TestEnumienum8, TestEnumienum9, TestEnumienum10,
                                     TestEnumienum11}; // Negative enumerators are not supported

        NSUInteger length = sizeof(values);

        TestEnumMutableShortEnumSeq* enumSeq1 = [NSMutableData dataWithBytes:values length:length];
        TestEnumMutableShortEnumSeq* enumSeq2 = [NSMutableData dataWithLength:length];
        @try
        {
            [proxy opIntSeq:enumSeq1 i2:&enumSeq2];
            test(NO);
        }
        @catch(ICEMarshalException*)
        {
        }
    }

    tprintf("ok\n");

    return proxy;
}
