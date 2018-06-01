// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        using(var communicator = initialize(ref args))
        {
            Test.MyClassPrx myClass = AllTests.allTests(this, false);
            Console.Out.Write("shutting down server... ");
            Console.Out.Flush();
            myClass.shutdown();
            Console.Out.WriteLine("ok");
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
