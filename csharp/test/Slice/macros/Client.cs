//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Slice.Test.Macros
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            Console.Out.Write("testing Slice predefined macros... ");
            Console.Out.Flush();
            var d = new Default();
            Assert(d.X == 10);
            Assert(d.Y == 10);

            var nd = new NoDefault();
            Assert(nd.X != 10);
            Assert(nd.Y != 10);

            var c = new CsOnly();
            Assert(c.Lang.Equals("cs"));
            Assert(c.Version == Ice.Runtime.IntVersion);
            Console.Out.WriteLine("ok");
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
