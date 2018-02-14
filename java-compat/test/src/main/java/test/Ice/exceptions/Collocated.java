// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

public class Collocated extends test.Util.Application
{
    @Override
    public int
    run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Object object = new ThrowerI();
        adapter.add(object, Ice.Util.stringToIdentity("thrower"));

        AllTests.allTests(this);

        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);
        // For this test, we need a dummy logger, otherwise the
        // assertion test will print an error message.
        //
        initData.logger = new DummyLogger();

        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.exceptions");
        initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0));

        return initData;
    }

    public static void
    main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main("Collocated", args);
        System.gc();
        System.exit(result);
    }
}
