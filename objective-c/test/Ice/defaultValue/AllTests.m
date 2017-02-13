// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    tprintf("testing default constructor... ");

    {
        TestDefaultValueStructNoDefaults* v = [TestDefaultValueStructNoDefaults structNoDefaults];
        test(v.bo == NO);
        test(v.b == 0);
        test(v.s == 0);
        test(v.i == 0);
        test(v.l == 0);
        test(v.f == 0.0);
        test(v.d == 0.0);
        test([v.str isEqual:@""]);
        test(v.c1 == TestDefaultValuered);
        test(v.bs == nil);
        test(v.is == nil);
        test(v.st != nil);
        test(v.dict == nil);

        TestDefaultValueExceptionNoDefaults* e = [TestDefaultValueExceptionNoDefaults exceptionNoDefaults];
        test([e.str isEqual:@""]);
        test(e.c1 == TestDefaultValuered);
        test(e.bs == nil);
        test(e.st != nil);
        test(e.dict == nil);

        TestDefaultValueClassNoDefaults* cl = [TestDefaultValueClassNoDefaults classNoDefaults];
        test([cl.str isEqual:@""]);
        test(cl.c1 == TestDefaultValuered);
        test(cl.bs == nil);
        test(cl.st != nil);
        test(cl.dict == nil);
    }
    tprintf("ok\n");
}
