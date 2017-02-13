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
        test(v.f == static_cast<float>(5.1));
        test(v.d == 6.2);
        test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07");
#ifdef ICE_CPP11_MAPPING
        test(v.c1 == Color::red);
        test(v.c2 == Color::green);
        test(v.c3 == Color::blue);
        test(v.nc1 == Nested::Color::red);
        test(v.nc2 == Nested::Color::green);
        test(v.nc3 == Nested::Color::blue);
#else
        test(v.c1 == red);
        test(v.c2 == green);
        test(v.c3 == blue);
        test(v.nc1 == Nested::red);
        test(v.nc2 == Nested::green);
        test(v.nc3 == Nested::blue);
#endif
        test(v.noDefault.empty());
        test(v.zeroI == 0);
        test(v.zeroL == 0);
        test(v.zeroF == 0);
        test(v.zeroDotF == 0);
        test(v.zeroD == 0);
        test(v.zeroDotD == 0);
    }

    {
        Struct2 v;
        test(v.boolTrue == ConstBool);
        test(v.b == ConstByte);
        test(v.s == ConstShort);
        test(v.i == ConstInt);
        test(v.l == ConstLong);
        test(v.f == ConstFloat);
        test(v.d == ConstDouble);
        test(v.str == ConstString);
        test(v.c1 == ConstColor1);
        test(v.c2 == ConstColor2);
        test(v.c3 == ConstColor3);
        test(v.nc1 == ConstNestedColor1);
        test(v.nc2 == ConstNestedColor2);
        test(v.nc3 == ConstNestedColor3);
        test(v.zeroI == ConstZeroI);
        test(v.zeroL == ConstZeroL);
        test(v.zeroF == ConstZeroF);
        test(v.zeroDotF == ConstZeroDotF);
        test(v.zeroD == ConstZeroD);
        test(v.zeroDotD == ConstZeroDotD);
    }

    {
#ifdef ICE_CPP11_MAPPING
        Struct3 v;
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == static_cast<float>(5.1));
        test(v.d == 6.2);
        test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07");
        test(v.c1 == Color::red);
        test(v.c2 == Color::green);
        test(v.c3 == Color::blue);
        test(v.nc1 == Nested::Color::red);
        test(v.nc2 == Nested::Color::green);
        test(v.nc3 == Nested::Color::blue);
        test(v.noDefault.empty());
        test(v.zeroI == 0);
        test(v.zeroL == 0);
        test(v.zeroF == 0);
        test(v.zeroDotF == 0);
        test(v.zeroD == 0);
        test(v.zeroDotD == 0);
#else
        Struct3Ptr v = new Struct3;
        test(!v->boolFalse);
        test(v->boolTrue);
        test(v->b == 1);
        test(v->s == 2);
        test(v->i == 3);
        test(v->l == 4);
        test(v->f == static_cast<float>(5.1));
        test(v->d == 6.2);
        test(v->str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07");
        test(v->c1 == red);
        test(v->c2 == green);
        test(v->c3 == blue);
        test(v->nc1 == Nested::red);
        test(v->nc2 == Nested::green);
        test(v->nc3 == Nested::blue);
        test(v->noDefault.empty());
        test(v->zeroI == 0);
        test(v->zeroL == 0);
        test(v->zeroF == 0);
        test(v->zeroDotF == 0);
        test(v->zeroD == 0);
        test(v->zeroDotD == 0);
#endif
    }

    {
        BasePtr v = ICE_MAKE_SHARED(Base);
        test(!v->boolFalse);
        test(v->boolTrue);
        test(v->b == 1);
        test(v->s == 2);
        test(v->i == 3);
        test(v->l == 4);
        test(v->f == static_cast<float>(5.1));
        test(v->d == 6.2);
        test(v->str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07");
        test(v->noDefault.empty());
        test(v->zeroI == 0);
        test(v->zeroL == 0);
        test(v->zeroF == 0);
        test(v->zeroDotF == 0);
        test(v->zeroD == 0);
        test(v->zeroDotD == 0);
    }

    {
        DerivedPtr v = ICE_MAKE_SHARED(Derived);
        test(!v->boolFalse);
        test(v->boolTrue);
        test(v->b == 1);
        test(v->s == 2);
        test(v->i == 3);
        test(v->l == 4);
        test(v->f == static_cast<float>(5.1));
        test(v->d == 6.2);
        test(v->str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07");
        test(v->noDefault.empty());
#ifdef ICE_CPP11_MAPPING
        test(v->c1 == Color::red);
        test(v->c2 == Color::green);
        test(v->c3 == Color::blue);
        test(v->nc1 == Nested::Color::red);
        test(v->nc2 == Nested::Color::green);
        test(v->nc3 == Nested::Color::blue);
#else
        test(v->c1 == red);
        test(v->c2 == green);
        test(v->c3 == blue);
        test(v->nc1 == Nested::red);
        test(v->nc2 == Nested::green);
        test(v->nc3 == Nested::blue);
#endif
        test(v->zeroI == 0);
        test(v->zeroL == 0);
        test(v->zeroF == 0);
        test(v->zeroDotF == 0);
        test(v->zeroD == 0);
        test(v->zeroDotD == 0);
    }

    {
        BaseEx v;
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == static_cast<float>(5.1));
        test(v.d == 6.2);
        test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07");
        test(v.noDefault.empty());
        test(v.zeroI == 0);
        test(v.zeroL == 0);
        test(v.zeroF == 0);
        test(v.zeroDotF == 0);
        test(v.zeroD == 0);
        test(v.zeroDotD == 0);
    }

    {
        DerivedEx v;
        test(!v.boolFalse);
        test(v.boolTrue);
        test(v.b == 1);
        test(v.s == 2);
        test(v.i == 3);
        test(v.l == 4);
        test(v.f == static_cast<float>(5.1));
        test(v.d == 6.2);
        test(v.str == "foo \\ \"bar\n \r\n\t\v\f\a\b? \007 \x07");
        test(v.noDefault.empty());
#ifdef ICE_CPP11_MAPPING
        test(v.c1 == Color::red);
        test(v.c2 == Color::green);
        test(v.c3 == Color::blue);
        test(v.nc1 == Nested::Color::red);
        test(v.nc2 == Nested::Color::green);
        test(v.nc3 == Nested::Color::blue);
#else
        test(v.c1 == red);
        test(v.c2 == green);
        test(v.c3 == blue);
        test(v.nc1 == Nested::red);
        test(v.nc2 == Nested::green);
        test(v.nc3 == Nested::blue);
#endif
        test(v.zeroI == 0);
        test(v.zeroL == 0);
        test(v.zeroF == 0);
        test(v.zeroDotF == 0);
        test(v.zeroD == 0);
        test(v.zeroDotD == 0);
    }

    cout << "ok" << endl;
}
