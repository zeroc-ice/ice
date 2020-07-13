//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Invoke
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            IMyClassPrx myClass = AllTests.allTests(this);
            myClass.shutdown();
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
