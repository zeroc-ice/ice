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
    namespace seqMapping
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties.setProperty("Ice.Package.Test", "Ice.seqMapping");
                using(var communicator = initialize(properties))
                {
                    var myClass = AllTests.allTests(this, false);
                    Console.Out.Write("shutting down server... ");
                    Console.Out.Flush();
                    myClass.shutdown();
                    Console.Out.WriteLine("ok");
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
