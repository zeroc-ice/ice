// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interrupt;

public class Server extends test.Util.Application
{
    @Override
    public int
    run(String[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");

        TestControllerI controller = new TestControllerI(adapter);
        adapter.add(new TestI(controller), communicator().stringToIdentity("test"));
        adapter.activate();
        adapter2.add(controller, communicator().stringToIdentity("testController"));
        adapter2.activate();

        return WAIT;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.interrupt");
        //
        // We need to enable the background IO so that Ice is interrupt
        // safe for this test.
        //
        initData.properties.setProperty("Ice.BackgroundIO", "1");
        //
        // We need to send messages large enough to cause the transport
        // buffers to fill up.
        //
        initData.properties.setProperty("Ice.MessageSizeMax", "20000");

        initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        initData.properties.setProperty("ControllerAdapter.Endpoints", "tcp -p 12011");
        initData.properties.setProperty("ControllerAdapter.ThreadPool.Size", "1");
        return initData;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
