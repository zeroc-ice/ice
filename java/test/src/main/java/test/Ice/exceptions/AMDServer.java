// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

public class AMDServer extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        com.zeroc.Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
        com.zeroc.Ice.ObjectAdapter adapter3 = communicator.createObjectAdapter("TestAdapter3");
        com.zeroc.Ice.Object object = new AMDThrowerI();
        adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("thrower"));
        adapter2.add(object, com.zeroc.Ice.Util.stringToIdentity("thrower"));
        adapter3.add(object, com.zeroc.Ice.Util.stringToIdentity("thrower"));
        adapter.activate();
        adapter2.activate();
        adapter3.activate();
        return WAIT;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);

        //
        // For this test, we need a dummy logger, otherwise the
        // assertion test will print an error message.
        //
        r.initData.logger = new DummyLogger();

        r.initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.exceptions.AMD");
        r.initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(r.initData.properties, 0) + ":udp");
        r.initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        r.initData.properties.setProperty("TestAdapter2.Endpoints", getTestEndpoint(r.initData.properties, 1));
        r.initData.properties.setProperty("TestAdapter2.MessageSizeMax", "0");
        r.initData.properties.setProperty("TestAdapter3.Endpoints", getTestEndpoint(r.initData.properties, 2));
        r.initData.properties.setProperty("TestAdapter3.MessageSizeMax", "1");

        return r;
    }

    public static void main(String[] args)
    {
        AMDServer app = new AMDServer();
        int result = app.main("AMDServer", args);
        System.gc();
        System.exit(result);
    }
}
