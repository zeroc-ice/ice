// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.facets;

public class Collocated extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010");
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        com.zeroc.Ice.Object d = new DI();
        adapter.add(d, com.zeroc.Ice.Util.stringToIdentity("d"));
        adapter.addFacet(d, com.zeroc.Ice.Util.stringToIdentity("d"), "facetABCD");
        com.zeroc.Ice.Object f = new FI();
        adapter.addFacet(f, com.zeroc.Ice.Util.stringToIdentity("d"), "facetEF");
        com.zeroc.Ice.Object h = new HI(communicator);
        adapter.addFacet(h, com.zeroc.Ice.Util.stringToIdentity("d"), "facetGH");

        AllTests.allTests(communicator, getWriter());

        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.facets");
        return r;
    }

    public static void main(String[] args)
    {
        Collocated app = new Collocated();
        int result = app.main("Collocated", args);
        System.gc();
        System.exit(result);
    }
}
