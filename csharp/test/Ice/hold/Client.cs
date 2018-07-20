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
    namespace hold
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Package.Test", "Ice.hold");
                using(var communicator = initialize(properties))
                {
                    AllTests.allTests(this);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
