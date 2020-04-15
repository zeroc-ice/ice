//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace Ice.operations
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.ThreadPool.Client.Size"] = "2";
            properties["Ice.ThreadPool.Client.SizeWarn"] = "0";
            using var communicator = Initialize(properties, typeIdNamespaces: new string[] { "Ice.operations.TypeId" });
            var myClass = AllTests.allTests(this);

            Console.Out.Write("testing server shutdown... ");
            Console.Out.Flush();
            myClass.shutdown();
            try
            {
                myClass.Clone(connectionTimeout: 100).IcePing(); // Use timeout to speed up testing on Windows
                Assert(false);
            }
            catch (Exception)
            {
                 Console.Out.WriteLine("ok");
            }
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
