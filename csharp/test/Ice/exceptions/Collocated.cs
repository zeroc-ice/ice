// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

namespace Ice
{
    namespace exceptions
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                Ice.Properties properties = createTestProperties(ref args);
                properties.setProperty("Ice.Warn.Connections", "0");
                properties.setProperty("Ice.Warn.Dispatch", "0");
                properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
                properties.setProperty("Ice.Package.Test", "Ice.exceptions");
                using(var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.Object obj = new ThrowerI();
                    adapter.add(obj, Ice.Util.stringToIdentity("thrower"));
                    AllTests.allTests(this);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
