// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
                Ice.Properties properties = createTestProperties(ref args);
                properties.setProperty("Ice.ThreadPool.Client.Size", "2");
                properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
                properties.setProperty("Ice.BatchAutoFlushSize", "100");
                properties.setProperty("Ice.Package.Test", "Ice.operations");
                using (var communicator = initialize(properties))
                {
                    var myClass = AllTests.allTests(this);

                    Console.Out.Write("testing server shutdown... ");
                    Console.Out.Flush();
                    myClass.shutdown();
                    try
                    {
                        myClass.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
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
