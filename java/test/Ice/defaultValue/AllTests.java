// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.defaultValue;

import java.io.PrintWriter;
import test.Ice.defaultValue.Test.*;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(test.Util.Application app, PrintWriter out)
    {
        out.print("testing default values... ");
        out.flush();

        {
            Struct1 v = new Struct1();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == (byte)254);
            test(v.s == 16000);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str.equals("foo bar"));
            test(v.c == Color.red);
            test(v.noDefault == null);
        }

        {
            Struct2 v = new Struct2();
            test(v.boolTrue == ConstBool.value);
            test(v.b == ConstByte.value);
            test(v.s == ConstShort.value);
            test(v.i == ConstInt.value);
            test(v.l == ConstLong.value);
            test(v.f == ConstFloat.value);
            test(v.d == ConstDouble.value);
            test(v.str.equals(ConstString.value));
            test(v.c == ConstColor.value);
        }

        {
            Base v = new Base();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str.equals("foo bar"));
            test(v.noDefault == null);
        }

        {
            Derived v = new Derived();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str.equals("foo bar"));
            test(v.c == Color.green);
            test(v.noDefault == null);
        }

        {
            BaseEx v = new BaseEx();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str == "foo bar");
            test(v.noDefault == null);
        }

        {
            DerivedEx v = new DerivedEx();
            test(!v.boolFalse);
            test(v.boolTrue);
            test(v.b == 1);
            test(v.s == 2);
            test(v.i == 3);
            test(v.l == 4);
            test(v.f == 5.1F);
            test(v.d == 6.2);
            test(v.str == "foo bar");
            test(v.noDefault == null);
            test(v.c == Color.green);
        }

        out.println("ok");
    }
}
