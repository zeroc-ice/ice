//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.DictMapping
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            System.IO.TextWriter output = GetWriter();
            IMyClassPrx myClass = AllTests.allTests(this, false);
            output.Write("shutting down server... ");
            output.Flush();
            myClass.shutdown();
            output.WriteLine("ok");
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
