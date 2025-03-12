// Copyright (c) ZeroC, Inc.

using Ice.defaultValue.Test;

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
                var v = new Struct1();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 254);
                test(v.s == 16000);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == (float)5.1);
                test(v.d == 6.2);
                test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                test(v.c1 == Color.red);
                test(v.c2 == Color.green);
                test(v.c3 == Color.blue);
                test(v.nc1 == Test.Nested.Color.red);
                test(v.nc2 == Test.Nested.Color.green);
                test(v.nc3 == Test.Nested.Color.blue);
                test(v.noDefault.Length == 0);
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            {
                var v = new Struct2();
                test(v.boolTrue == ConstBool.value);
                test(v.b == ConstByte.value);
                test(v.s == ConstShort.value);
                test(v.i == ConstInt.value);
                test(v.l == ConstLong.value);
                test(v.f == ConstFloat.value);
                test(v.d == ConstDouble.value);
                test(v.str.Equals(ConstString.value));
                test(v.c1 == ConstColor1.value);
                test(v.c2 == ConstColor2.value);
                test(v.c3 == ConstColor3.value);
                test(v.nc1 == ConstNestedColor1.value);
                test(v.nc2 == ConstNestedColor2.value);
                test(v.nc3 == ConstNestedColor3.value);
                test(v.zeroI == ConstZeroI.value);
                test(v.zeroL == ConstZeroL.value);
                test(v.zeroF == ConstZeroF.value);
                test(v.zeroDotF == ConstZeroDotF.value);
                test(v.zeroD == ConstZeroD.value);
                test(v.zeroDotD == ConstZeroDotD.value);
            }

            {
                var v = new Struct3();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 1);
                test(v.s == 2);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == 5.1F);
                test(v.d == 6.2);
                // test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                // test(v.str == "foo bar");
                test(v.c1 == Color.red);
                test(v.c2 == Color.green);
                test(v.c3 == Color.blue);
                test(v.nc1 == Test.Nested.Color.red);
                test(v.nc2 == Test.Nested.Color.green);
                test(v.nc3 == Test.Nested.Color.blue);
                // test(v.noDefault == null);
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            {
                var v = new Struct4();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 1);
                test(v.s == 2);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == 5.1F);
                test(v.d == 6.2);
                // test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                // test(v.str == "foo bar");
                test(v.c1 == Color.red);
                test(v.c2 == Color.green);
                test(v.c3 == Color.blue);
                test(v.nc1 == Test.Nested.Color.red);
                test(v.nc2 == Test.Nested.Color.green);
                test(v.nc3 == Test.Nested.Color.blue);
                // test(v.noDefault == null);
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            {
                var v = new Base();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 1);
                test(v.s == 2);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == 5.1F);
                test(v.d == 6.2);
                test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                test(v.noDefault.Length == 0);
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            {
                var v = new Derived();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 1);
                test(v.s == 2);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == 5.1F);
                test(v.d == 6.2);
                test(v.str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                test(v.c1 == Color.red);
                test(v.c2 == Color.green);
                test(v.c3 == Color.blue);
                test(v.nc1 == Test.Nested.Color.red);
                test(v.nc2 == Test.Nested.Color.green);
                test(v.nc3 == Test.Nested.Color.blue);
                test(v.noDefault.Length == 0);
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            {
                var v = new BaseEx();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 1);
                test(v.s == 2);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == 5.1F);
                test(v.d == 6.2);
                test(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
                test(v.noDefault.Length == 0);
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            {
                var v = new DerivedEx();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 1);
                test(v.s == 2);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == 5.1F);
                test(v.d == 6.2);
                test(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
                test(v.noDefault.Length == 0);
                test(v.c1 == Color.red);
                test(v.c2 == Color.green);
                test(v.c3 == Color.blue);
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
                var v = new ClassProperty();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 1);
                test(v.s == 2);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == 5.1F);
                test(v.d == 6.2);
                test(v.str == "foo bar");
                test(v.noDefault.Length == 0);
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            {
                var v = new StructProperty();
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
                var v = new ExceptionProperty();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b == 1);
                test(v.s == 2);
                test(v.i == 3);
                test(v.l == 4);
                test(v.f == 5.1F);
                test(v.d == 6.2);
                test(v.str == "foo bar");
                test(v.noDefault.Length == 0);
                test(v.zeroI == 0);
                test(v.zeroL == 0);
                test(v.zeroF == 0);
                test(v.zeroDotF == 0);
                test(v.zeroD == 0);
                test(v.zeroDotD == 0);
            }

            output.WriteLine("ok");

            output.Write("testing non-primary constructor... ");
            output.Flush();
            {
                var v = new StructNoDefaults(bs: [], iseq: [], st2: new(), dict: []);
                test(v.bo == false);
                test(v.b == 0);
                test(v.s == 0);
                test(v.i == 0);
                test(v.l == 0);
                test(v.f == 0.0);
                test(v.d == 0.0);
                test(v.str.Length == 0);
                test(v.c1 == Color.red);
                test(v.bs.Length == 0);
                test(v.iseq.Length == 0);
                test(v.st.a == 0);
                test(v.st2.a.Length == 0);
                test(v.dict.Count == 0);

                var e = new ExceptionNoDefaults(bs: [], st2: new(), dict: []);
                test(e.str.Length == 0);
                test(e.c1 == Color.red);
                test(e.bs.Length == 0);
                test(e.st.a == 0);
                test(e.st2.a.Length == 0);
                test(e.dict.Count == 0);

                var cl = new ClassNoDefaults(bs: [], st2: new(), dict: []);
                test(cl.str.Length == 0);
                test(cl.c1 == Color.red);
                test(e.bs.Length == 0);
                test(cl.st.a == 0);
                test(e.st2.a.Length == 0);
                test(e.dict.Count == 0);
            }
            output.WriteLine("ok");
        }
    }
}
