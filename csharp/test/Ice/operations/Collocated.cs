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
        public class Collocated : TestHelper
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
                    communicator.getProperties().setProperty("TestAdapter.AdapterId", "test");
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.ObjectPrx prx = adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
                    //adapter.activate(); // Don't activate OA to ensure collocation is used.

                    if (prx.ice_getConnection() != null)
                    {
                        throw new System.Exception();
                    }

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
