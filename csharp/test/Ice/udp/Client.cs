//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
                using (var communicator = initialize(properties))
                {
                    AllTests.allTests(this);

                    int num;
                    try
                    {
                        num = args.Length == 1 ? Int32.Parse(args[0]) : 1;
                    }
                    catch (FormatException)
                    {
                        num = 1;
                    }

                    for (int i = 0; i < num; ++i)
                    {
                        Test.TestIntfPrx.Parse("control:" + getTestEndpoint(i, "tcp"), communicator).shutdown();
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
