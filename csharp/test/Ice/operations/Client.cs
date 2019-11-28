//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace Ice
{
    namespace operations
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[] { "Ice.operations.TypeId" };
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
                initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
                using (var communicator = initialize(initData))
                {
                    var myClass = AllTests.allTests(this);

                    Console.Out.Write("testing server shutdown... ");
                    Console.Out.Flush();
                    myClass.shutdown();
                    try
                    {
                        myClass.Clone(connectionTimeout: 100).IcePing(); // Use timeout to speed up testing on Windows
                        test(false);
                    }
                    catch (Ice.LocalException)
                    {
                        Console.Out.WriteLine("ok");
                    }
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
