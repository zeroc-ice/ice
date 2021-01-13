// Copyright (c) ZeroC, Inc. All rights reserved.

using System.IO;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.DefaultValue
{
    public static class AllTests
    {
        public static Task RunAsync(TestHelper helper)
        {
            TextWriter output = helper.Output;
            output.Write("testing default values... ");
            output.Flush();

            {
                var v = new Base();
                TestHelper.Assert(!v.BoolFalse);
                TestHelper.Assert(v.BoolTrue);
                TestHelper.Assert(v.B == 1);
                TestHelper.Assert(v.S == 2);
                TestHelper.Assert(v.I == 3);
                TestHelper.Assert(v.L == 4);
                TestHelper.Assert(v.F == 5.1F);
                TestHelper.Assert(v.D == 6.2);
                TestHelper.Assert(v.Str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                TestHelper.Assert(v.NoDefault == null);
                TestHelper.Assert(v.ZeroI == 0);
                TestHelper.Assert(v.ZeroL == 0);
                TestHelper.Assert(v.ZeroF == 0);
                TestHelper.Assert(v.ZeroDotF == 0);
                TestHelper.Assert(v.ZeroD == 0);
                TestHelper.Assert(v.ZeroDotD == 0);
            }

            {
                var v = new Derived();
                TestHelper.Assert(!v.BoolFalse);
                TestHelper.Assert(v.BoolTrue);
                TestHelper.Assert(v.B == 1);
                TestHelper.Assert(v.S == 2);
                TestHelper.Assert(v.I == 3);
                TestHelper.Assert(v.L == 4);
                TestHelper.Assert(v.F == 5.1F);
                TestHelper.Assert(v.D == 6.2);
                TestHelper.Assert(v.Str.Equals("foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007"));
                TestHelper.Assert(v.C1 == Color.red);
                TestHelper.Assert(v.C2 == Color.green);
                TestHelper.Assert(v.C3 == Color.blue);
                TestHelper.Assert(v.Nc1 == Nested.Color.red);
                TestHelper.Assert(v.Nc2 == Nested.Color.green);
                TestHelper.Assert(v.Nc3 == Nested.Color.blue);
                TestHelper.Assert(v.NoDefault == null);
                TestHelper.Assert(v.ZeroI == 0);
                TestHelper.Assert(v.ZeroL == 0);
                TestHelper.Assert(v.ZeroF == 0);
                TestHelper.Assert(v.ZeroDotF == 0);
                TestHelper.Assert(v.ZeroD == 0);
                TestHelper.Assert(v.ZeroDotD == 0);
            }

            {
                var v = new BaseEx();
                TestHelper.Assert(!v.BoolFalse);
                TestHelper.Assert(v.BoolTrue);
                TestHelper.Assert(v.B == 1);
                TestHelper.Assert(v.S == 2);
                TestHelper.Assert(v.I == 3);
                TestHelper.Assert(v.L == 4);
                TestHelper.Assert(v.F == 5.1F);
                TestHelper.Assert(v.D == 6.2);
                TestHelper.Assert(v.Str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
                TestHelper.Assert(v.NoDefault == null);
                TestHelper.Assert(v.ZeroI == 0);
                TestHelper.Assert(v.ZeroL == 0);
                TestHelper.Assert(v.ZeroF == 0);
                TestHelper.Assert(v.ZeroDotF == 0);
                TestHelper.Assert(v.ZeroD == 0);
                TestHelper.Assert(v.ZeroDotD == 0);
            }

            {
                var v = new DerivedEx();
                TestHelper.Assert(!v.BoolFalse);
                TestHelper.Assert(v.BoolTrue);
                TestHelper.Assert(v.B == 1);
                TestHelper.Assert(v.S == 2);
                TestHelper.Assert(v.I == 3);
                TestHelper.Assert(v.L == 4);
                TestHelper.Assert(v.F == 5.1F);
                TestHelper.Assert(v.D == 6.2);
                TestHelper.Assert(v.Str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b? \u0007 \u0007");
                TestHelper.Assert(v.NoDefault == null);
                TestHelper.Assert(v.C1 == Color.red);
                TestHelper.Assert(v.C2 == Color.green);
                TestHelper.Assert(v.C3 == Color.blue);
                TestHelper.Assert(v.Nc1 == Nested.Color.red);
                TestHelper.Assert(v.Nc2 == Nested.Color.green);
                TestHelper.Assert(v.Nc3 == Nested.Color.blue);
                TestHelper.Assert(v.ZeroI == 0);
                TestHelper.Assert(v.ZeroL == 0);
                TestHelper.Assert(v.ZeroF == 0);
                TestHelper.Assert(v.ZeroDotF == 0);
                TestHelper.Assert(v.ZeroD == 0);
                TestHelper.Assert(v.ZeroDotD == 0);
            }

            output.WriteLine("ok");
            return Task.CompletedTask;
        }
    }
}
