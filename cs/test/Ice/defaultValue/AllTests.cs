// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

#if SILVERLIGHT
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
#endif

public class AllTests : TestCommon.TestApp
{
#if SILVERLIGHT
    override
    public void run(Ice.Communicator communicator)
#else
    public static void allTests()
#endif
    {
        Write("testing default values... ");
        Flush();

        {
            Test.Struct1 v = new Test.Struct1();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 254);
            test(v.s == 16000);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == (float)5.1);
            test(v.d == 6.2);
            test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
            test(v.c1 == Test.Color.red);
            test(v.c2 == Test.Color.green);
            test(v.c3 == Test.Color.blue);
            test(v.nc1 == Test.Nested.Color.red);
            test(v.nc2 == Test.Nested.Color.green);
            test(v.nc3 == Test.Nested.Color.blue);
            test(v.noDefault == null);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.Struct2 v = new Test.Struct2();
            test(v.boolTrue == Test.ConstBool.value);
            test(v.b == Test.ConstByte.value);
            test(v.s == Test.ConstShort.value);
            test(v.i == Test.ConstInt.value);
            test(v.l == Test.ConstLong.value);
            test(v.f == Test.ConstFloat.value);
            test(v.d == Test.ConstDouble.value);
            test(v.str.Equals(Test.ConstString.value));
            test(v.c1 == Test.ConstColor1.value);
            test(v.c2 == Test.ConstColor2.value);
            test(v.c3 == Test.ConstColor3.value);
            test(v.nc1 == Test.ConstNestedColor1.value);
            test(v.nc2 == Test.ConstNestedColor2.value);
            test(v.nc3 == Test.ConstNestedColor3.value);
            test(v.zeroI == Test.ConstZeroI.value);
            test(v.zeroL == Test.ConstZeroL.value);
            test(v.zeroF == Test.ConstZeroF.value);
            test(v.zeroDotF == Test.ConstZeroDotF.value);
            test(v.zeroD == Test.ConstZeroD.value);
            test(v.zeroDotD == Test.ConstZeroDotD.value);
        }

        {
            Test.Struct3 v = new Test.Struct3();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            //test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
            //test(v.str.Equals("foo bar"));
            test(v.c1 == Test.Color.red);
            test(v.c2 == Test.Color.green);
            test(v.c3 == Test.Color.blue);
            test(v.nc1 == Test.Nested.Color.red);
            test(v.nc2 == Test.Nested.Color.green);
            test(v.nc3 == Test.Nested.Color.blue);
            //test(v.noDefault == null);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.Struct4 v = new Test.Struct4();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            //test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
            //test(v.str.Equals("foo bar"));
            test(v.c1 == Test.Color.red);
            test(v.c2 == Test.Color.green);
            test(v.c3 == Test.Color.blue);
            test(v.nc1 == Test.Nested.Color.red);
            test(v.nc2 == Test.Nested.Color.green);
            test(v.nc3 == Test.Nested.Color.blue);
            //test(v.noDefault == null);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.Base v = new Test.Base();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
            test(v.noDefault == null);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.Derived v = new Test.Derived();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
            test(v.c1 == Test.Color.red);
            test(v.c2 == Test.Color.green);
            test(v.c3 == Test.Color.blue);
            test(v.nc1 == Test.Nested.Color.red);
            test(v.nc2 == Test.Nested.Color.green);
            test(v.nc3 == Test.Nested.Color.blue);
            test(v.noDefault == null);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.BaseEx v = new Test.BaseEx();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
            test(v.noDefault == null);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.DerivedEx v = new Test.DerivedEx();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
            test(v.noDefault == null);
            test(v.c1 == Test.Color.red);
            test(v.c2 == Test.Color.green);
            test(v.c3 == Test.Color.blue);
            test(v.nc1 == Test.Nested.Color.red);
            test(v.nc2 == Test.Nested.Color.green);
            test(v.nc3 == Test.Nested.Color.blue);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.ClassProperty v = new Test.ClassProperty();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str.Equals("foo bar"));
            test(v.noDefault == null);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.StructProperty v = new Test.StructProperty();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        {
            Test.ExceptionProperty v = new Test.ExceptionProperty();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str.Equals("foo bar"));
            test(v.noDefault == null);
            test(v.zeroI == 0);
            test(v.zeroL == 0);
            test(v.zeroF == 0);
            test(v.zeroDotF == 0);
            test(v.zeroD == 0);
            test(v.zeroDotD == 0);
        }

        WriteLine("ok");
    }
}
