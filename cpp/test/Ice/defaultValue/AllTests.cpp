// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

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
    }

    {
        BasePtr v = std::make_shared<Base>();
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
        DerivedPtr v = std::make_shared<Derived>();
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
        test(v->c1 == Color::red);
        test(v->c2 == Color::green);
        test(v->c3 == Color::blue);
        test(v->nc1 == Nested::Color::red);
        test(v->nc2 == Nested::Color::green);
        test(v->nc3 == Nested::Color::blue);
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
        test(v.c1 == Color::red);
        test(v.c2 == Color::green);
        test(v.c3 == Color::blue);
        test(v.nc1 == Nested::Color::red);
        test(v.nc2 == Nested::Color::green);
        test(v.nc3 == Nested::Color::blue);
        test(v.zeroI == 0);
        test(v.zeroL == 0);
        test(v.zeroF == 0);
        test(v.zeroDotF == 0);
        test(v.zeroD == 0);
        test(v.zeroDotD == 0);
    }

    cout << "ok" << endl;
}
