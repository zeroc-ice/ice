// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <DefaultValueTest.h>

#import <Foundation/Foundation.h>

void
defaultValueAllTests()
{
    tprintf("testing default values... ");

    {
        TestDefaultValueStruct1* v = [TestDefaultValueStruct1 struct1];
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 254);
        test(v.s == 16000);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test([v.str isEqualToString:@"foo bar"]);
        test(v.c == TestDefaultValuered);
        test([v.noDefault length] == 0);
    }

    {
        TestDefaultValueStruct2* v = [TestDefaultValueStruct2 struct2];
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test([v.str isEqualToString:@"foo bar"]);
        test(v.c == TestDefaultValueblue);
        test([v.noDefault length] == 0);
    }

    {
        TestDefaultValueBase* v = [TestDefaultValueBase base];
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test([v.str isEqualToString:@"foo bar"]);
        test([v.noDefault length] == 0);
    }

    {
        TestDefaultValueDerived* v = [TestDefaultValueDerived derived];
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test([v.str isEqualToString:@"foo bar"]);
        test([v.noDefault length] == 0);
        test(v.c == TestDefaultValuegreen);
    }

    {
        TestDefaultValueBaseEx* v = [TestDefaultValueBaseEx baseEx];
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test([v.str isEqualToString:@"foo bar"]);
        test([v.noDefault length] == 0);
    }

    {
        TestDefaultValueDerivedEx* v = [TestDefaultValueDerivedEx derivedEx];
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test([v.str isEqualToString:@"foo bar"]);
        test([v.noDefault length] == 0);
        test(v.c == TestDefaultValuegreen);
    }

    tprintf("ok\n");
}
