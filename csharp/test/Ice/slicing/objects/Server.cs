// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class Server : TestCommon.Application
{
    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + " -t 2000");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.Object obj = new TestI();
        adapter.add(obj, Ice.Util.stringToIdentity("Test"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
