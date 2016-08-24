// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.seqMapping;

public class Collocated extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        java.io.PrintWriter out = getWriter();
        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.add(new MyClassI(), com.zeroc.Ice.Util.stringToIdentity("test"));
        //adapter.activate(); // Don't activate OA to ensure collocation is used.

        AllTests.allTests(communicator(), true, out);

        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.seqMapping");
        r.initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        return r;
    }

    public static void main(String[] args)
    {
        Collocated c = new Collocated();
        int status = c.main("Collocated", args);

        System.gc();
        System.exit(status);
    }
}
