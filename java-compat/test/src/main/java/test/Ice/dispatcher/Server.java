// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.dispatcher;

public class Server extends test.Util.Application
{
    @Override
    public int
    run(String[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");

        assert(_dispatcher != null);
        adapter.add(new TestI(_dispatcher), Ice.Util.stringToIdentity("test"));
        adapter.activate();
        adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
        adapter2.activate();

        return WAIT;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        assert(_dispatcher == null);
        _dispatcher = new Dispatcher();
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.dispatcher");
        initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        initData.properties.setProperty("ControllerAdapter.Endpoints", "tcp -p 12011");
        initData.properties.setProperty("ControllerAdapter.ThreadPool.Size", "1");
        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties.setProperty("Ice.TCP.RcvSize", "50000");
        initData.dispatcher = _dispatcher;
        return initData;
    }

    public void terminate()
    {
        if(_dispatcher != null)
        {
            _dispatcher.terminate();
        }
    }

    public static void
    main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        app.terminate();
        System.gc();
        System.exit(result);
    }

    //
    // The Dispatcher class uses a static "_instance" member in other language
    // mappings. In Java, we avoid the use of static members because we need to
    // maintain support for Android (in which the client and server run in the
    // same process).
    //
    private Dispatcher _dispatcher;
}
