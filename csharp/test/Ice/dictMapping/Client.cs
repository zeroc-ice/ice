//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.DictMapping
{
    public class Client : TestHelper
    {
        public override Task Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            System.IO.TextWriter output = GetWriter();
            IMyClassPrx myClass = AllTests.allTests(this, false);
            output.Write("shutting down server... ");
            output.Flush();
            myClass.shutdown();
            output.WriteLine("ok");
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
