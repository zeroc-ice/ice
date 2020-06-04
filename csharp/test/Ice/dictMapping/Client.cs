//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.DictMapping
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            using Communicator communicator = Initialize(ref args);
            System.IO.TextWriter output = GetWriter();
            IMyClassPrx myClass = AllTests.allTests(this, false);
            output.Write("shutting down server... ");
            output.Flush();
            myClass.shutdown();
            output.WriteLine("ok");
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
