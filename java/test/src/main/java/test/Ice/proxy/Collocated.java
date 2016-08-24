// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
        com.zeroc.Ice.Communicator communicator = communicator();
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010");
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MyDerivedClassI(), com.zeroc.Ice.Util.stringToIdentity("test"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        AllTests.allTests(communicator, getWriter());

        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.proxy");
        r.initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        return r;
    }

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main(" extends", args);
        System.gc();
        System.exit(result);
    }
}
