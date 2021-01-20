// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Slice.Test.Macros
{
    public static class Client
    {
        public static Task<int> Main()
        {
            int status = 0;
            try
            {
                Console.Out.Write("testing Slice predefined macros... ");
                Console.Out.Flush();
                var d = new Default();
                TestHelper.Assert(d.X == 10);
                TestHelper.Assert(d.Y == 10);

                var nd = new NoDefault();
                TestHelper.Assert(nd.X != 10);
                TestHelper.Assert(nd.Y != 10);

                var c = new CsOnly();
                TestHelper.Assert(c.Lang.Equals("cs"));
                TestHelper.Assert(c.Version == Ice.Runtime.IntVersion);
                Console.Out.WriteLine("ok");
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                status = 1;
            }
            return Task.FromResult(status);
        }
    }
}
