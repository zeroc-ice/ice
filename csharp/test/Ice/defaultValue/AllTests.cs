//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.IO;
using Test;

namespace ZeroC.Ice.Test.DefaultValue
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            TextWriter output = helper.GetWriter();
            output.Write("testing default values... ");
            output.Flush();

            {
                var v = new Base();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                TestHelper.Assert(v.noDefault == null);
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new Derived();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                TestHelper.Assert(v.c1 == Color.red);
                TestHelper.Assert(v.c2 == Color.green);
                TestHelper.Assert(v.c3 == Color.blue);
                TestHelper.Assert(v.nc1 == Nested.Color.red);
                TestHelper.Assert(v.nc2 == Nested.Color.green);
                TestHelper.Assert(v.nc3 == Nested.Color.blue);
                TestHelper.Assert(v.noDefault == null);
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new BaseEx();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
                TestHelper.Assert(v.noDefault == null);
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new DerivedEx();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
                TestHelper.Assert(v.noDefault == null);
                TestHelper.Assert(v.c1 == Color.red);
                TestHelper.Assert(v.c2 == Color.green);
                TestHelper.Assert(v.c3 == Color.blue);
                TestHelper.Assert(v.nc1 == Nested.Color.red);
                TestHelper.Assert(v.nc2 == Nested.Color.green);
                TestHelper.Assert(v.nc3 == Nested.Color.blue);
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new ClassProperty();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str.Equals("foo bar"));
                TestHelper.Assert(v.noDefault == null);
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new ExceptionProperty();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str.Equals("foo bar"));
                TestHelper.Assert(v.noDefault == null);
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            output.WriteLine("ok");
        }
    }
}
