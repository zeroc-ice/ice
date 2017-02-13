// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.adapterDeactivation;

public class Collocated extends test.Util.Application
{
    public int run(String[] args)
    {
        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        com.zeroc.Ice.ServantLocator locator = new ServantLocatorI();
        adapter.addServantLocator(locator, "");

        AllTests.allTests(this);

        adapter.waitForDeactivate();
        return 0;
    }

    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);

        //
        // 2 threads are necessary to dispatch the collocated transient() call with AMI
        //
        initData.properties.setProperty("TestAdapter.ThreadPool.Size", "2");

        initData.properties.setProperty("Ice.Package.Test", "test.Ice.adapterDeactivation");
        initData.properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(initData.properties, 0));
        return initData;
    }

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main("Collocated", args);
        System.exit(result);
    }
}
