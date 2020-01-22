//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.defaultValue
{
    public class AllTests : global::Test.AllTests
    {
        public static void allTests(global::Test.TestHelper helper)
        {
            var output = helper.getWriter();
            output.Write("testing default values... ");
            output.Flush();

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
                test(v.noDefault.Equals(""));
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
                test(v.noDefault.Equals(""));
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
                test(v.noDefault.Equals(""));
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
                test(v.noDefault.Equals(""));
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
                test(v.noDefault.Equals(""));
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
                test(v.noDefault.Equals(""));
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            output.WriteLine("ok");

            output.Write("testing default constructor... ");
            output.Flush();
            {
                Test.ExceptionNoDefaults e = new Test.ExceptionNoDefaults();
                test(e.str.Equals(""));
                test(e.c1 == Test.Color.red);
                test(e.bs == null);
                test(e.st.a == 0);
                test(e.st2 != null);
                test(e.dict == null);

                Test.ClassNoDefaults cl = new Test.ClassNoDefaults();
                test(cl.str.Equals(""));
                test(cl.c1 == Test.Color.red);
                test(cl.bs == null);
                test(cl.st.a == 0);
                test(cl.st2 != null);
                test(cl.dict == null);
            }
            output.WriteLine("ok");
        }
    }
}
