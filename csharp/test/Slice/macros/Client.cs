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
        public override Task Run(string[] args)
        {
            Console.Out.Write("testing Slice predefined macros... ");
            Console.Out.Flush();
            var d = new Default();
            Assert(d.x == 10);
            Assert(d.y == 10);

            var nd = new NoDefault();
            Assert(nd.x != 10);
            Assert(nd.y != 10);

            var c = new CsOnly();
            Assert(c.lang.Equals("cs"));
            Assert(c.version == Ice.Runtime.IntVersion);
            Console.Out.WriteLine("ok");
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
