// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <StreamTest.h>

static int
run(id<ICECommunicator> communicator)
{
    id<ICEInputStream> in;
    id<ICEOutputStream> out;
    NSMutableData* data;

    //
    // Test the stream api.
    //
    tprintf("testing primitive types... ");

    {
        NSData* byte = [NSData data];
        in = [ICEUtil createInputStream:communicator data:byte];
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out startEncapsulation];
        [out writeBool:YES];
        [out endEncapsulation];
        data = [out finished];
        NSData* d = [out finishedNoCopy];
        test([d isEqual:data]);
        out = nil;

        in = [ICEUtil createInputStream:communicator data:data];
        [in startEncapsulation];
        BOOL v;
        v = [in readBool];
        test(v);
        [in endEncapsulation];
    }

    {
        NSData* byte = [NSData data];
        in = [ICEUtil createInputStream:communicator data:byte];
        @try
        {
            BOOL v;
            v = [in readBool];
            test(NO);
        }
        @catch(ICEUnmarshalOutOfBoundsException* ex)
        {
        }
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out writeBool:YES];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        BOOL v = [in readBool];
        test(v);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out writeByte:(ICEByte)1];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEByte v = [in readByte];
        test(v == 1);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out writeShort:(ICEShort)2];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEShort v = [in readShort];
        test(v == 2);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out writeInt:(ICEInt)3];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEInt v;
        v = [in readInt];
        test(v == 3);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out writeLong:(ICELong)4];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICELong v;
        v = [in readLong];
        test(v == 4);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out writeFloat:(ICEFloat)5.0];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEFloat v;
        v = [in readFloat];
        test(v == 5.0);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out writeDouble:(ICEDouble)6.0];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEDouble v;
        v = [in readDouble];
        test(v == 6.0);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [out writeString:@"hello world"];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        NSString* v;
        v = [in readString];
        test([v isEqualToString:@"hello world"]);
    }

    tprintf("ok\n");

    tprintf("testing constructed types... ");

    {
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamMyEnumHelper write:@(TestStreamenum3) stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        id e = [TestStreamMyEnumHelper read:in];
        test([e isEqual:@(TestStreamenum3)]);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStreamSmallStruct* s = [TestStreamSmallStruct smallStruct];
        s.bo = YES;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = @"7";
        s.e = TestStreamenum2;
        s.p = [TestStreamMyClassPrx uncheckedCast:[communicator stringToProxy:@"test:default"]];
        [TestStreamSmallStructHelper write:s stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamSmallStruct* s2 = [TestStreamSmallStructHelper read:in];
        test([s2 isEqual:s]);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStreamClassStruct* s = [TestStreamClassStruct classStruct];
        s.i = 10;
        [TestStreamClassStructHelper write:s stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamClassStruct* s2 = [TestStreamClassStructHelper read:in];
        test(s2.i == s.i);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStreamOptionalClass* o = [TestStreamOptionalClass optionalClass];
        o.bo = NO;
        o.by = 5;
        o.sh = 4;
        o.i = 3;
        [TestStreamOptionalClassHelper write:o stream:out];
        [out writePendingValues];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamOptionalClass* ICE_AUTORELEASING_QUALIFIER o2;
        [TestStreamOptionalClassHelper read:&o2 stream:in];
        [in readPendingValues];
        test(o2.bo == o.bo);
        test(o2.by == o.by);
        if([in getEncoding] == ICEEncoding_1_0)
        {
            test(![o2 hasSh]);
            test(![o2 hasI]);
        }
        else
        {
            test(o2.sh == o.sh);
            test(o2.i == o.i);
        }
    }

    {
        out = [ICEUtil createOutputStream:communicator encoding:ICEEncoding_1_0];
        TestStreamOptionalClass* o = [TestStreamOptionalClass optionalClass];
        o.bo = NO;
        o.by = 5;
        o.sh = 4;
        o.i = 3;
        [TestStreamOptionalClassHelper write:o stream:out];
        [out writePendingValues];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data encoding:ICEEncoding_1_0];
        TestStreamOptionalClass* ICE_AUTORELEASING_QUALIFIER o2;
        [TestStreamOptionalClassHelper read:&o2 stream:in];
        [in readPendingValues];
        test(o2.bo == o.bo);
        test(o2.by == o.by);
        test(![o2 hasSh]);
        test(![o2 hasI]);
    }

    {
        BOOL buf[] = { YES, YES, NO, YES };
        ICEBoolSeq* arr = [ICEBoolSeq dataWithBytes:buf length:sizeof(buf)];

        out = [ICEUtil createOutputStream:communicator];
        [ICEBoolSeqHelper write:arr stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        ICEBoolSeq* arr2 = [ICEBoolSeqHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableBoolSS* arrS = [TestStreamMutableBoolSS array];
        [arrS addObject:arr];
        [arrS addObject:[ICEBoolSeq data]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamBoolSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamBoolSS* arr2S = [TestStreamBoolSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        ICEByte buf[] = { 0x01, 0x11, 0x12, 0x22 };
        ICEByteSeq* arr = [ICEByteSeq dataWithBytes:buf length:sizeof(buf)];

        out = [ICEUtil createOutputStream:communicator];
        [ICEByteSeqHelper write:arr stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEByteSeq* arr2 = [ICEByteSeqHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableByteSS* arrS = [TestStreamMutableByteSS array];
        [arrS addObject:arr];
        [arrS addObject:[ICEByteSeq data]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamByteSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamByteSS* arr2S = [TestStreamByteSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        ICEShort buf[] = { 1, 11, 12, 22 };
        ICEShortSeq* arr = [ICEShortSeq dataWithBytes:buf length:sizeof(buf)];

        out = [ICEUtil createOutputStream:communicator];
        [ICEShortSeqHelper write:arr stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEShortSeq* arr2 = [ICEShortSeqHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableShortSS* arrS = [TestStreamMutableShortSS array];
        [arrS addObject:arr];
        [arrS addObject:[ICEShortSeq data]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamShortSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamShortSS* arr2S = [TestStreamShortSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        ICEInt buf[] = { 1, 11, 12, 22 };
        ICEIntSeq* arr = [ICEIntSeq dataWithBytes:buf length:sizeof(buf)];

        out = [ICEUtil createOutputStream:communicator];
        [ICEIntSeqHelper write:arr stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEIntSeq* arr2 = [ICEIntSeqHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableIntSS* arrS = [TestStreamMutableIntSS array];
        [arrS addObject:arr];
        [arrS addObject:[ICEIntSeq data]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamIntSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamIntSS* arr2S = [TestStreamIntSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        ICELong buf[] = { 1, 11, 12, 22 };
        ICELongSeq* arr = [ICELongSeq dataWithBytes:buf length:sizeof(buf)];

        out = [ICEUtil createOutputStream:communicator];
        [ICELongSeqHelper write:arr stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICELongSeq* arr2 = [ICELongSeqHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableLongSS* arrS = [TestStreamMutableLongSS array];
        [arrS addObject:arr];
        [arrS addObject:[ICELongSeq data]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamLongSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamLongSS* arr2S = [TestStreamLongSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        ICEFloat buf[] = { 1, 2, 3, 4 };
        ICEFloatSeq* arr = [ICEFloatSeq dataWithBytes:buf length:sizeof(buf)];

        out = [ICEUtil createOutputStream:communicator];
        [ICEFloatSeqHelper write:arr stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEFloatSeq* arr2 = [ICEFloatSeqHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableFloatSS* arrS = [TestStreamMutableFloatSS array];
        [arrS addObject:arr];
        [arrS addObject:[ICEFloatSeq data]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamFloatSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamFloatSS* arr2S = [TestStreamFloatSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        ICEDouble buf[] = { 1, 2, 3, 4 };
        ICEDoubleSeq* arr = [ICEDoubleSeq dataWithBytes:buf length:sizeof(buf)];

        out = [ICEUtil createOutputStream:communicator];
        [ICEDoubleSeqHelper write:arr stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEDoubleSeq* arr2 = [ICEDoubleSeqHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableDoubleSS* arrS = [TestStreamMutableDoubleSS array];
        [arrS addObject:arr];
        [arrS addObject:[ICEDoubleSeq data]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamDoubleSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamDoubleSS* arr2S = [TestStreamDoubleSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        ICEMutableStringSeq* arr = [ICEMutableStringSeq array];
        [arr addObject:@"string1"];
        [arr addObject:@"string2"];
        [arr addObject:@"string3"];
        [arr addObject:@"string4"];
        out = [ICEUtil createOutputStream:communicator];
        [ICEStringSeqHelper write:arr stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        ICEStringSeq* arr2 = [ICEStringSeqHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableStringSS* arrS = [TestStreamMutableStringSS array];
        [arrS addObject:arr];
        [arrS addObject:[ICEStringSeq array]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamStringSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamStringSS* arr2S = [TestStreamStringSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        TestStreamMyEnum buf[] = { TestStreamenum3, TestStreamenum2, TestStreamenum1, TestStreamenum2 };
        TestStreamMutableMyEnumS* arr = [TestStreamMutableMyEnumS dataWithBytes:buf length:sizeof(buf)];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamMyEnumSHelper write:arr stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamMyEnumS* arr2 = [TestStreamMyEnumSHelper read:in];
        test([arr2 isEqual:arr]);

        TestStreamMutableMyEnumSS* arrS = [TestStreamMutableMyEnumSS array];
        [arrS addObject:arr];
        [arrS addObject:[TestStreamMyEnumS data]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamMyEnumSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamMyEnumSS* arr2S = [TestStreamMyEnumSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        TestStreamMutableSmallStructS* arr = [TestStreamMutableSmallStructS array];
        for(int i = 0; i < 4; ++i)
        {
            TestStreamSmallStruct* s = [TestStreamSmallStruct smallStruct];
            s.bo = YES;
            s.by = 1;
            s.sh = 2;
            s.i = 3;
            s.l = 4;
            s.f = 5.0;
            s.d = 6.0;
            s.str = @"7";
            s.e = TestStreamenum2;
            s.p = [TestStreamMyClassPrx uncheckedCast:[communicator stringToProxy:@"test:default"]];
            [arr addObject:s];
        }
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamSmallStructSHelper write:arr stream:out];
        [out writePendingValues];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamSmallStructS* arr2 = [TestStreamSmallStructSHelper read:in];
        [in readPendingValues];
        test([arr2 count] == [arr count]);
        for(int j = 0; j < (int)[arr2 count]; ++j)
        {
            test([[arr objectAtIndex:j] isEqual:[arr2 objectAtIndex:j]]);
        }

        TestStreamMutableSmallStructSS* arrS = [TestStreamMutableSmallStructSS array];
        [arrS addObject:arr];
        [arrS addObject:[TestStreamSmallStructS array]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamSmallStructSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamSmallStructSS* arr2S = [TestStreamSmallStructSSHelper read:in];
        test([arr2S isEqual:arrS]);
    }

    {
        TestStreamMutableMyClassS* arr = [TestStreamMutableMyClassS array];
        for(int i = 0; i < 4; ++i)
        {
            TestStreamMyClass* c = [TestStreamMyClass myClass];
            c.c = c;
            c.o = c;
            c.s = [TestStreamSmallStruct smallStruct];
            c.s.e = TestStreamenum2;

            BOOL boolS[] = { YES, NO, YES, NO };
            c.seq1 = [NSMutableData dataWithBytes:boolS length:sizeof(boolS)];

            ICEByte byteS[] = { 1, 2, 3, 4 };
            c.seq2 = [NSMutableData dataWithBytes:byteS length:sizeof(byteS)];

            ICEShort shortS[] = { 1, 2, 3, 4 };
            c.seq3 = [NSMutableData dataWithBytes:shortS length:sizeof(shortS)];

            ICEInt intS[] = { 1, 2, 3, 4 };
            c.seq4 = [NSMutableData dataWithBytes:intS length:sizeof(intS)];

            ICELong longS[] = { 1, 2, 3, 4 };
            c.seq5 = [NSMutableData dataWithBytes:longS length:sizeof(longS)];

            ICEFloat floatS[] = { 1, 2, 3, 4 };
            c.seq6 = [NSMutableData dataWithBytes:floatS length:sizeof(floatS)];

            ICEDouble doubleS[] = { 1, 2, 3, 4 };
            c.seq7 = [NSMutableData dataWithBytes:doubleS length:sizeof(doubleS)];

            c.seq8 = [ICEMutableStringSeq array];
            [(ICEMutableStringSeq*)c.seq8 addObject:@"string1"];
            [(ICEMutableStringSeq*)c.seq8 addObject:@"string2"];
            [(ICEMutableStringSeq*)c.seq8 addObject:@"string3"];
            [(ICEMutableStringSeq*)c.seq8 addObject:@"string4"];

            TestStreamMyEnum enumS[] = { TestStreamenum3, TestStreamenum2, TestStreamenum1 };
            c.seq9 = [NSMutableData dataWithBytes:enumS length:sizeof(enumS)];

            c.d = [NSDictionary dictionaryWithObject:[TestStreamMyClass myClass] forKey:@"hi"];
            [arr addObject:c];
        }
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamMyClassSHelper write:arr stream:out];
        [out writePendingValues];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamMyClassS* arr2 = [TestStreamMyClassSHelper read:in];
        [in readPendingValues];
        test([arr2 count] > 0);
        test([arr2 count] == [arr count]);
        for(int j = 0; j < (int)[arr2 count]; ++j)
        {
            TestStreamMyClass* e = [arr2 objectAtIndex:j];
            TestStreamMyClass* f = [arr objectAtIndex:j];
            test(e);
            test(e.c == e);
            test(e.o == e);
            test(e.s.e == TestStreamenum2);
            test([e.seq1 isEqual:f.seq1]);
            test([e.seq2 isEqual:f.seq2]);
            test([e.seq3 isEqual:f.seq3]);
            test([e.seq4 isEqual:f.seq4]);
            test([e.seq5 isEqual:f.seq5]);
            test([e.seq6 isEqual:f.seq6]);
            test([e.seq7 isEqual:f.seq7]);
            test([e.seq8 isEqual:f.seq8]);
            test([e.seq9 isEqual:f.seq9]);
            test([e.d objectForKey:@"hi"] != nil);

            e.c = nil;
            e.o = nil;
            f.c = nil;
            f.o = nil;
        }

        TestStreamMutableMyClassSS* arrS = [TestStreamMutableMyClassSS array];
        [arrS addObject:arr];
        [arrS addObject:[TestStreamMyClassS array]];
        [arrS addObject:arr];

        out = [ICEUtil createOutputStream:communicator];
        [TestStreamMyClassSSHelper write:arrS stream:out];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamMyClassSS* arr2S = [TestStreamMyClassSSHelper read:in];
        test([arr2S count] == [arrS count]);
    }

    {
        TestStreamMyInterface ICE_AUTORELEASING_QUALIFIER * i = ICE_AUTORELEASE([TestStreamMyInterface new]);
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamMyInterfaceHelper write:i stream:out];
        [out writePendingValues];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        i = nil;
        [TestStreamMyInterfaceHelper read:&i stream:in];
        [in readPendingValues];
        test(i != nil);
    }

    //
    // ObjectWriter/ObjectReader not supported.
    //
    // {
    //     out = [ICEUtil createOutputStream:communicator];
    //     TestStreamMyClass* obj = [TestStreamMyClass myClass];
    //     obj.s.e = TestStreamenum2;
    //     TestObjectWriterPtr writer = new TestObjectWriter:obj];
    //     [out writeObject:writer];
    //     [out writePendingValues];
    //     data = [out finished];
    //     test([writer called);
    // }

    // {
    //     out = [ICEUtil createOutputStream:communicator];
    //     TestStreamMyClassPtr obj = new TestStreamMyClass;
    //     obj->s.e = TestStreamenum2;
    //     TestObjectWriterPtr writer = new TestObjectWriter:obj];
    //     [out writeObject:writer];
    //     [out writePendingValues];
    //     data = [out finished];
    //     test([writer called]);
    //     [factoryWrapper setFactory:new TestObjectFactory];
    //     in = [ICEUtil createInputStream:communicator data:data];
    //     TestReadObjectCallbackPtr cb = new TestReadObjectCallback;
    //     [in readObject:cb];
    //     [in readPendingValues];
    //     test([cb obj]);
    //     TestObjectReaderPtr reader = TestObjectReaderPtr::dynamicCast(cb->obj);
    //     test(reader);
    //     test([reader called]);
    //     test([reader obj]);
    //     test([reader obj]->s.e == TestStreamenum2);
    //     [factoryWrapper setFactory:0];
    // }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStreamMyException* ex = [TestStreamMyException myException];
        TestStreamMyClass* c = [TestStreamMyClass myClass];
        c.c = c;
        c.o = c;
        c.s = [TestStreamSmallStruct smallStruct];
        c.s.e = TestStreamenum2;

        BOOL boolS[] = { YES, NO, YES, NO };
        c.seq1 = [NSMutableData dataWithBytes:boolS length:sizeof(boolS)];

        ICEByte byteS[] = { 1, 2, 3, 4 };
        c.seq2 = [NSMutableData dataWithBytes:byteS length:sizeof(byteS)];

        ICEShort shortS[] = { 1, 2, 3, 4 };
        c.seq3 = [NSMutableData dataWithBytes:shortS length:sizeof(shortS)];

        ICEInt intS[] = { 1, 2, 3, 4 };
        c.seq4 = [NSMutableData dataWithBytes:intS length:sizeof(intS)];

        ICELong longS[] = { 1, 2, 3, 4 };
        c.seq5 = [NSMutableData dataWithBytes:longS length:sizeof(longS)];

        ICEFloat floatS[] = { 1, 2, 3, 4 };
        c.seq6 = [NSMutableData dataWithBytes:floatS length:sizeof(floatS)];

        ICEDouble doubleS[] = { 1, 2, 3, 4 };
        c.seq7 = [NSMutableData dataWithBytes:doubleS length:sizeof(doubleS)];

        c.seq8 = [ICEMutableStringSeq array];
        [(ICEMutableStringSeq*)c.seq8 addObject:@"string1"];
        [(ICEMutableStringSeq*)c.seq8 addObject:@"string2"];
        [(ICEMutableStringSeq*)c.seq8 addObject:@"string3"];
        [(ICEMutableStringSeq*)c.seq8 addObject:@"string4"];

        TestStreamMyEnum enumS[] = { TestStreamenum3, TestStreamenum2, TestStreamenum1 };
        c.seq9 = [NSMutableData dataWithBytes:enumS length:sizeof(enumS)];

        c.d = [NSDictionary dictionaryWithObject:[TestStreamMyClass myClass] forKey:@"hi"];

        ex.c = c;

        [out writeException:ex];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        @try
        {
            [in throwException];
            test(NO);
        }
        @catch(TestStreamMyException* ex1)
        {
            test(ex1.c);
            test(ex1.c.c == ex1.c);
            test(ex1.c.o == ex1.c);
            test(ex1.c.s.e == TestStreamenum2);
            test([ex1.c.seq1 isEqual:c.seq1]);
            test([ex1.c.seq2 isEqual:c.seq2]);
            test([ex1.c.seq3 isEqual:c.seq3]);
            test([ex1.c.seq4 isEqual:c.seq4]);
            test([ex1.c.seq5 isEqual:c.seq5]);
            test([ex1.c.seq6 isEqual:c.seq6]);
            test([ex1.c.seq7 isEqual:c.seq7]);
            test([ex1.c.seq8 isEqual:c.seq8]);
            test([ex1.c.seq9 isEqual:c.seq9]);
            test([ex1.c.d objectForKey:@"hi"] != nil);

            ex1.c.c = nil;
            ex1.c.o = nil;
            c.c = nil;
            c.o = nil;
        }
    }

    {
        TestStreamMutableByteBoolD* dict = [TestStreamMutableByteBoolD dictionary];
        [dict setObject:@YES forKey:@(0x04)];
        [dict setObject:@NO forKey:@(0x01)];
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamByteBoolDHelper write:dict stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamByteBoolD* dict2 = [TestStreamByteBoolDHelper read:in];
        test([dict2 isEqual:dict]);
    }

    {
        TestStreamMutableShortIntD* dict = [TestStreamMutableShortIntD dictionary];
        [dict setObject:@9 forKey:@1];
        [dict setObject:@8 forKey:@4];
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamShortIntDHelper write:dict stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamShortIntD* dict2 = [TestStreamShortIntDHelper read:in];
        test([dict2 isEqual:dict]);
    }

    {
        TestStreamMutableLongFloatD* dict = [TestStreamMutableLongFloatD dictionary];
        [dict setObject:@0.51f forKey:@123809828];
        [dict setObject:@0.56f forKey:@123809829];
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamLongFloatDHelper write:dict stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamLongFloatD* dict2 = [TestStreamLongFloatDHelper read:in];
        test([dict2 isEqual:dict]);
    }

    {
        TestStreamMutableStringStringD* dict = [TestStreamMutableStringStringD dictionary];
        [dict setObject:@"value1" forKey:@"key1"];
        [dict setObject:@"value2" forKey:@"key2"];
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamStringStringDHelper write:dict stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamStringStringD* dict2 = [TestStreamStringStringDHelper read:in];
        test([dict2 isEqual:dict]);
    }

    {
        TestStreamMutableStringMyClassD* dict = [TestStreamMutableStringMyClassD dictionary];
        TestStreamMyClass* c = [TestStreamMyClass myClass];
        c.s = [TestStreamSmallStruct smallStruct];
        c.s.e = TestStreamenum2;
        [dict setObject:c forKey:@"key1"];
        c = [TestStreamMyClass myClass];
        c.s = [TestStreamSmallStruct smallStruct];
        c.s.e = TestStreamenum3;
        [dict setObject:c forKey:@"key2"];
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamStringMyClassDHelper write:dict stream:out];
        [out writePendingValues];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamStringMyClassD* dict2 = [TestStreamStringMyClassDHelper read:in];
        [in readPendingValues];
        test([dict2 count] == [dict count]);
        test([dict2 objectForKey:@"key1"] != nil &&
             ((TestStreamMyClass*)[dict2 objectForKey:@"key1"]).s.e == TestStreamenum2);
        test([dict2 objectForKey:@"key2"] != nil &&
             ((TestStreamMyClass*)[dict2 objectForKey:@"key2"]).s.e == TestStreamenum3);

    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [TestStreamSubNestedEnumHelper write:@(TestStreamSubnestedEnum3) stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        id e = [TestStreamSubNestedEnumHelper read:in];
        test([e isEqual:@(TestStreamSubnestedEnum3)]);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStreamSubNestedStruct* s = [TestStreamSubNestedStruct nestedStruct];
        s.bo = YES;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = @"7";
        s.e = TestStreamSubnestedEnum2;
        [TestStreamSubNestedStructHelper write:s stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamSubNestedStruct* s2 = [TestStreamSubNestedStructHelper read:in];
        test([s2 isEqual:s]);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStreamSubNestedClassStruct* s = [TestStreamSubNestedClassStruct nestedClassStruct];
        s.i = 10;
        [TestStreamSubNestedClassStructHelper write:s stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStreamSubNestedClassStruct* s2 = [TestStreamSubNestedClassStructHelper read:in];
        test(s2.i == s.i);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStreamSubNestedException* ex = [TestStreamSubNestedException nestedException];
        ex.str = @"str";

        [out writeException:ex];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        @try
        {
            [in throwException];
            test(NO);
        }
        @catch(TestStreamSubNestedException* ex1)
        {
            test([ex1.str isEqualToString:ex.str]);
        }
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        [TestStream2Sub2NestedEnum2Helper write:@(TestStream2Sub2nestedEnum4) stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        id e = [TestStream2Sub2NestedEnum2Helper read:in];
        test([e isEqual:@(TestStream2Sub2nestedEnum4)]);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStream2Sub2NestedStruct2* s = [TestStream2Sub2NestedStruct2 nestedStruct2];
        s.bo = YES;
        s.by = 1;
        s.sh = 2;
        s.i = 3;
        s.l = 4;
        s.f = 5.0;
        s.d = 6.0;
        s.str = @"7";
        s.e = TestStream2Sub2nestedEnum5;
        [TestStream2Sub2NestedStruct2Helper write:s stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStream2Sub2NestedStruct2* s2 = [TestStream2Sub2NestedStruct2Helper read:in];
        test([s2 isEqual:s]);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStream2Sub2NestedClassStruct2* s = [TestStream2Sub2NestedClassStruct2 nestedClassStruct2];
        s.i = 10;
        [TestStream2Sub2NestedClassStruct2Helper write:s stream:out];
        data = [out finished];
        in = [ICEUtil createInputStream:communicator data:data];
        TestStream2Sub2NestedClassStruct2* s2 = [TestStream2Sub2NestedClassStruct2Helper read:in];
        test(s2.i == s.i);
    }

    {
        out = [ICEUtil createOutputStream:communicator];
        TestStream2Sub2NestedException2* ex = [TestStream2Sub2NestedException2 nestedException2];
        ex.str = @"str";

        [out writeException:ex];
        data = [out finished];

        in = [ICEUtil createInputStream:communicator data:data];
        @try
        {
            [in throwException];
            test(NO);
        }
        @catch(TestStream2Sub2NestedException2* ex1)
        {
            test([ex1.str isEqualToString:ex.str]);
        }
    }

    tprintf("ok\n");
    return 0;
}

#if TARGET_OS_IPHONE
#  define main streamClient
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
    ICEregisterIceWS(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    ICEregisterIceIAP(YES);
#endif
#endif

    int status;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {

            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultClientProperties(&argc, argv);
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestStream", @"::Test",
                                      @"TestStreamSub", @"::Test::Sub",
                                      @"TestStream2", @"::Test2",
                                      @"TestStream2Sub2", @"::Test2::Sub2",
                                      nil];
#endif
            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(communicator);
        }
        @catch(ICEException* ex)
        {
            tprintf("%@\n", ex);
            status = EXIT_FAILURE;
        }
        @catch(TestFailedException* ex)
        {
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            [communicator destroy];
        }
    }
    return status;
}
