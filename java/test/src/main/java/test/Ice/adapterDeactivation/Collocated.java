// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

        AllTests.allTests(this, getWriter());

        adapter.waitForDeactivate();
        return 0;
    }

    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);

        //
        // 2 threads are necessary to dispatch the collocated transient() call with AMI
        //
        r.initData.properties.setProperty("TestAdapter.ThreadPool.Size", "2");

        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.adapterDeactivation");
        r.initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        return r;
    }

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main("Collocated", args);
        System.exit(result);
    }
}
