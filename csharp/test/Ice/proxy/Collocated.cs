// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

public class Collocated : TestCommon.Application
{
    public override int run(string[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.add(new MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        AllTests.allTests(this);

        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2"); // For nested AMI.
        initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        return initData;
    }

    public static int Main(String[] args)
    {
        Collocated app = new Collocated();
        return app.runmain(args);
    }
}
