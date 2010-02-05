// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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

void
allTests()
{
    cout << "testing default values... " << flush;

    {
        Struct1 v;
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 254);
        test(v.s == 16000);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test(v.str == "foo bar");
        test(v.c == red);
        test(v.noDefault.empty());
    }

    {
        Struct2Ptr v = new Struct2;
        test(!v->boolFalse);
        test(v->boolTrue);
        test(v->b == 1);
        test(v->s == 2);
        test(v->i == 3);
        test(v->l == 4);
        test(v->f == 5.0);
        test(v->d == 6.0);
        test(v->str == "foo bar");
        test(v->c == blue);
        test(v->noDefault.empty());
    }

    {
        BasePtr v = new Base;
        test(!v->boolFalse);
        test(v->boolTrue);
        test(v->b == 1);
        test(v->s == 2);
        test(v->i == 3);
        test(v->l == 4);
        test(v->f == 5.0);
        test(v->d == 6.0);
        test(v->str == "foo bar");
        test(v->noDefault.empty());
    }

    {
        DerivedPtr v = new Derived;
        test(!v->boolFalse);
        test(v->boolTrue);
        test(v->b == 1);
        test(v->s == 2);
        test(v->i == 3);
        test(v->l == 4);
        test(v->f == 5.0);
        test(v->d == 6.0);
        test(v->str == "foo bar");
        test(v->noDefault.empty());
        test(v->c == green);
    }

    {
        BaseEx v;
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test(v.str == "foo bar");
        test(v.noDefault.empty());
    }

    {
        DerivedEx v;
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == 5.0);
        test(v.d == 6.0);
        test(v.str == "foo bar");
        test(v.noDefault.empty());
        test(v.c == green);
    }

    cout << "ok" << endl;
}
