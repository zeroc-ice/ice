//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.IO;
using Test;

namespace Ice.defaultValue
{
    public class AllTests
    {
        public static void allTests(TestHelper helper)
        {
            TextWriter output = helper.GetWriter();
            output.Write("testing default values... ");
            output.Flush();

            {
                var v = new Test.Base();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                TestHelper.Assert(v.noDefault.Equals(""));
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new Test.Derived();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                TestHelper.Assert(v.c1 == Test.Color.red);
                TestHelper.Assert(v.c2 == Test.Color.green);
                TestHelper.Assert(v.c3 == Test.Color.blue);
                TestHelper.Assert(v.nc1 == Test.Nested.Color.red);
                TestHelper.Assert(v.nc2 == Test.Nested.Color.green);
                TestHelper.Assert(v.nc3 == Test.Nested.Color.blue);
                TestHelper.Assert(v.noDefault.Equals(""));
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new Test.BaseEx();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
                TestHelper.Assert(v.noDefault.Equals(""));
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new Test.DerivedEx();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
                TestHelper.Assert(v.noDefault.Equals(""));
                TestHelper.Assert(v.c1 == Test.Color.red);
                TestHelper.Assert(v.c2 == Test.Color.green);
                TestHelper.Assert(v.c3 == Test.Color.blue);
                TestHelper.Assert(v.nc1 == Test.Nested.Color.red);
                TestHelper.Assert(v.nc2 == Test.Nested.Color.green);
                TestHelper.Assert(v.nc3 == Test.Nested.Color.blue);
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new Test.ClassProperty();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str.Equals("foo bar"));
                TestHelper.Assert(v.noDefault.Equals(""));
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            {
                var v = new Test.ExceptionProperty();
                TestHelper.Assert(!v.boolFalse);
                TestHelper.Assert(v.boolTrue);
                TestHelper.Assert(v.b == 1);
                TestHelper.Assert(v.s == 2);
                TestHelper.Assert(v.i == 3);
                TestHelper.Assert(v.l == 4);
                TestHelper.Assert(v.f == 5.1F);
                TestHelper.Assert(v.d == 6.2);
                TestHelper.Assert(v.str.Equals("foo bar"));
                TestHelper.Assert(v.noDefault.Equals(""));
                TestHelper.Assert(v.zeroI == 0);
                TestHelper.Assert(v.zeroL == 0);
                TestHelper.Assert(v.zeroF == 0);
                TestHelper.Assert(v.zeroDotF == 0);
                TestHelper.Assert(v.zeroD == 0);
                TestHelper.Assert(v.zeroDotD == 0);
            }

            output.WriteLine("ok");

            output.Write("testing default constructor... ");
            output.Flush();
            {
                var e = new Test.ExceptionNoDefaults();
                TestHelper.Assert(e.str.Equals(""));
                TestHelper.Assert(e.c1 == Test.Color.red);
                TestHelper.Assert(e.bs.Length == 0);
                TestHelper.Assert(e.st.a == 0);
                TestHelper.Assert(e.st2 != null);
                TestHelper.Assert(e.dict.Count == 0);

                var cl = new Test.ClassNoDefaults();
                TestHelper.Assert(cl.str.Equals(""));
                TestHelper.Assert(cl.c1 == Test.Color.red);
                TestHelper.Assert(cl.bs.Length == 0);
                TestHelper.Assert(cl.st.a == 0);
                TestHelper.Assert(cl.st2 != null);
                TestHelper.Assert(cl.dict.Count == 0);
            }
            output.WriteLine("ok");
        }
    }
}
