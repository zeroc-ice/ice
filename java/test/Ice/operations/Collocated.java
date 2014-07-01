// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

public class Collocated extends test.Util.Application
{
    public int run(String[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp");
        java.io.PrintWriter out = getWriter();
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.ObjectPrx prx = adapter.add(new MyDerivedClassI(), communicator().stringToIdentity("test"));
        adapter.activate();

        if(prx.ice_getConnection() != null)
        {
            throw new RuntimeException();
        }

        AllTests.allTests(this, out);

        return 0;
    }

    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.operations");

        //
        // We must set MessageSizeMax to an explicit values,
        // because we run tests to check whether
        // Ice.MemoryLimitException is raised as expected.
        //
        initData.properties.setProperty("Ice.MessageSizeMax", "100");

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
