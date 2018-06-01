// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.SndSize", "16384");

        using(var communicator = initialize(properties))
        {
            AllTests.allTests(this);

            int num;
            try
            {
                num = args.Length == 1 ? Int32.Parse(args[0]) : 0;
            }
            catch(FormatException)
            {
                num = 0;
            }

            for(int i = 0; i < num; ++i)
            {
                Ice.ObjectPrx prx = communicator.stringToProxy("control:" + getTestEndpoint(i, "tcp"));
                TestIntfPrxHelper.uncheckedCast(prx).shutdown();
            }
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
