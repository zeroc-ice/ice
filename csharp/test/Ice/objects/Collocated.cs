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
    namespace objects
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                Ice.Properties properties = createTestProperties(ref args);
                properties.setProperty("Ice.Warn.Dispatch", "0");
                properties.setProperty("Ice.Package.Test", "Ice.objects");
                using(var communicator = initialize(properties))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    var initial = new InitialI(adapter);
                    adapter.add(initial, Ice.Util.stringToIdentity("initial"));
                    var uet = new UnexpectedObjectExceptionTestI();
                    adapter.add(uet, Ice.Util.stringToIdentity("uoet"));
                    Test.AllTests.allTests(this);
                    // We must call shutdown even in the collocated case for cyclic dependency cleanup
                    initial.shutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
