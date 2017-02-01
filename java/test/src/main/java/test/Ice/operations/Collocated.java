// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

public class Collocated extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + ":udp");
        java.io.PrintWriter out = getWriter();
        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        com.zeroc.Ice.ObjectPrx prx = adapter.add(new MyDerivedClassI(), com.zeroc.Ice.Util.stringToIdentity("test"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        if(prx.ice_getConnection() != null)
        {
            throw new RuntimeException();
        }

        AllTests.allTests(this);

        return 0;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        if(initData.properties.getPropertyAsInt("Ice.ThreadInterruptSafe") > 0 || isAndroid())
        {
            initData.properties.setProperty("Ice.ThreadPool.Server.Size", "2");
        }
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.operations");

        initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");

        //
        // Its possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we supress this warning.
        //
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        return initData;
    }

    public static void main(String[] args)
    {
        Collocated c = new Collocated();
        int status = c.main("Collocated", args);

        System.gc();
        System.exit(status);
    }
}
