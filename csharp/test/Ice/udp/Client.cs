// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

namespace Ice
{
    namespace udp
    {
        public class Client : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Warn.Connections", "0");
                properties.setProperty("Ice.UDP.SndSize", "16384");
                using(var communicator = initialize(properties))
                {
                    AllTests.allTests(this);

                    int num;
                    try
                    {
                        num = args.Length == 1 ? Int32.Parse(args[0]) : 1;
                    }
                    catch(FormatException)
                    {
                        num = 1;
                    }

                    for(int i = 0; i < num; ++i)
                    {
                        var prx = communicator.stringToProxy("control:" + getTestEndpoint(i, "tcp"));
                        Test.TestIntfPrxHelper.uncheckedCast(prx).shutdown();
                    }
                }
            }

            public static int Main(string[] args)
            {
                return global::Test.TestDriver.runTest<Client>(args);
            }
        }
    }
}
