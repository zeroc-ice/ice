// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }

    public static void
    allTests()
    {
        Console.Out.Write("testing default values... ");
        Console.Out.Flush();

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
            test(v.str.Equals("foo bar"));
            test(v.c == Test.Color.red);
            test(v.noDefault == null);
        }

        {
            Test.Struct2 v = new Test.Struct2();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == (float)5.1);
            test(v.d == 6.2);
            //test(v.str.Equals("foo bar"));
            test(v.c == Test.Color.blue);
            //test(v.noDefault == null);
        }

        {
            Test.Base v = new Test.Base();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == (float)5.1);
            test(v.d == 6.2);
            test(v.str.Equals("foo bar"));
            test(v.noDefault == null);
        }

        {
            Test.Derived v = new Test.Derived();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == (float)5.1);
            test(v.d == 6.2);
            test(v.str.Equals("foo bar"));
            test(v.c == Test.Color.green);
            test(v.noDefault == null);
        }

        {
            Test.BaseEx v = new Test.BaseEx();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == (float)5.1);
            test(v.d == 6.2);
            test(v.str == "foo bar");
            test(v.noDefault == null);
        }

        {
            Test.DerivedEx v = new Test.DerivedEx();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == (float)5.1);
            test(v.d == 6.2);
            test(v.str == "foo bar");
            test(v.noDefault == null);
            test(v.c == Test.Color.green);
        }

        Console.Out.WriteLine("ok");
    }
}
