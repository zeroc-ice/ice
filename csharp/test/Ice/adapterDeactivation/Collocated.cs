// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Collocated : Test.TestHelper
{
    public override void run(string[] args)
    {
        using(var communicator = initialize(ref args))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));

            //
            // 2 threads are necessary to dispatch the collocated transient() call with AMI
            //
            communicator.getProperties().setProperty("TestAdapter.ThreadPool.Size", "2");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.ServantLocator locator = new ServantLocatorI();
            adapter.addServantLocator(locator, "");

            AllTests.allTests(this);

            adapter.waitForDeactivate();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Collocated>(args);
    }
}
