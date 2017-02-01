// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        com.zeroc.Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");

        adapter.add(new TestI(), com.zeroc.Ice.Util.stringToIdentity("test"));
        adapter.activate();
        adapter2.add(new TestControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("testController"));
        adapter2.activate();

        return WAIT;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.ami");
        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0));
        initData.properties.setProperty("ControllerAdapter.Endpoints", getTestEndpoint(initData.properties, 1));
        initData.properties.setProperty("ControllerAdapter.ThreadPool.Size", "1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties.setProperty("Ice.TCP.RcvSize", "50000");
        return initData;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
