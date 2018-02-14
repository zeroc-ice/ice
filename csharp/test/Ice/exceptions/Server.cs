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

public sealed class DummyLogger : Ice.Logger
{
    public void print(string message)
    {
    }

    public void trace(string category, string message)
    {
    }

    public void warning(string message)
    {
    }

    public void error(string message)
    {
    }

    public string getPrefix()
    {
        return "";
    }

    public Ice.Logger cloneWithPrefix(string prefix)
    {
        return new DummyLogger();
    }
}

public class Server : TestCommon.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("TestAdapter2");
        Ice.ObjectAdapter adapter3 = communicator().createObjectAdapter("TestAdapter3");
        Ice.Object obj = new ThrowerI();
        adapter.add(obj, Ice.Util.stringToIdentity("thrower"));
        adapter2.add(obj, Ice.Util.stringToIdentity("thrower"));
        adapter3.add(obj, Ice.Util.stringToIdentity("thrower"));
        adapter.activate();
        adapter2.activate();
        adapter3.activate();
        communicator().waitForShutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0));
        initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        initData.properties.setProperty("TestAdapter2.Endpoints", getTestEndpoint(initData.properties, 1));
        initData.properties.setProperty("TestAdapter2.MessageSizeMax", "0");
        initData.properties.setProperty("TestAdapter3.Endpoints", getTestEndpoint(initData.properties, 2));
        initData.properties.setProperty("TestAdapter3.MessageSizeMax", "1");
        return initData;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
