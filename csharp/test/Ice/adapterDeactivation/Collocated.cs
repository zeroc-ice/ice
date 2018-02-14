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
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Collocated
{
    internal class App : Ice.Application
    {
        public override int run(string[] args)
        {
            communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010");

            //
            // 2 threads are necessary to dispatch the collocated transient() call with AMI
            //
            communicator().getProperties().setProperty("TestAdapter.ThreadPool.Size", "2");

            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            Ice.ServantLocator locator = new ServantLocatorI();
            adapter.addServantLocator(locator, "");

            AllTests.allTests(communicator());

            adapter.waitForDeactivate();
            return 0;
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args);
    }
}
