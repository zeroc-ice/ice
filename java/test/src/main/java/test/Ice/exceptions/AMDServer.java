// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);

        //
        // For this test, we need a dummy logger, otherwise the
        // assertion test will print an error message.
        //
        initData.logger = new DummyLogger();

        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.exceptions.AMD");
        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0) + ":udp");
        initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        initData.properties.setProperty("TestAdapter2.Endpoints", getTestEndpoint(initData.properties, 1));
        initData.properties.setProperty("TestAdapter2.MessageSizeMax", "0");
        initData.properties.setProperty("TestAdapter3.Endpoints", getTestEndpoint(initData.properties, 2));
        initData.properties.setProperty("TestAdapter3.MessageSizeMax", "1");

        return initData;
    }

    public static void main(String[] args)
    {
        AMDServer app = new AMDServer();
        int result = app.main("AMDServer", args);
        System.gc();
        System.exit(result);
    }
}
