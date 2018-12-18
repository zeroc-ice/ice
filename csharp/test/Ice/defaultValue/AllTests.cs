// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace defaultValue
    {
        public class AllTests : global::Test.AllTests
        {
            public static void allTests(global::Test.TestHelper helper)
            {
                var output = helper.getWriter();
                output.Write("testing default values... ");
                output.Flush();

                {
                    Test.Struct1 v = new Test.Struct1();
                    test(!v.boolFalse);
                    test(v.boolTrue);
                    test(v.b == 254);
                    test(v.s == 16000);
                    test(v.i == 3);
                    test(v.l == 4);
                    test(v.f ==(float)5.1);
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
                    Test.StructNoDefaults v = new Test.StructNoDefaults();
                    test(v.bo == false);
                    test(v.b == 0);
                    test(v.s == 0);
                    test(v.i == 0);
                    test(v.l == 0);
                    test(v.f == 0.0);
                    test(v.d == 0.0);
                    test(v.str.Equals(""));
                    test(v.c1 == Test.Color.red);
                    test(v.bs == null);
                    test(v.iseq == null);
                    test(v.st.a == 0);
                    test(v.st2 != null);
                    test(v.dict == null);

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
}
