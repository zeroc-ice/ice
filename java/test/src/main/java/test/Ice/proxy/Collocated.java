// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.proxy;

public class Collocated extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010");
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MyDerivedClassI(), communicator.stringToIdentity("test"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        AllTests.allTests(communicator, getWriter());

        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.proxy");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        return initData;
    }

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main(" extends", args);
        System.gc();
        System.exit(result);
    }
}
