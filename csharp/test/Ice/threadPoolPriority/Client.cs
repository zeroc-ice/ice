// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace threadPoolPriority
    {
        public class Client : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Package.Test", "Ice.threadPoolPriority");
                using(var communicator = initialize(properties))
                {
                    var output = getWriter();
                    output.Write("testing server priority... ");
                    output.Flush();
                    var obj = communicator.stringToProxy("test:" + getTestEndpoint(0) + " -t 10000");
                    Test.PriorityPrx priority = Test.PriorityPrxHelper.checkedCast(obj);
                    test("AboveNormal".Equals(priority.getPriority()));
                    output.WriteLine("ok");
                    priority.shutdown();
                }
            }

            public static int Main(string[] args)
            {
                return global::Test.TestDriver.runTest<Client>(args);
            }
        }
    }
}
