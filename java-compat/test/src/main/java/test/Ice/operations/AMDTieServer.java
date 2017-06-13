// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.AMD.Test._MyDerivedClassTie;

public class AMDTieServer extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        adapter.add(new _MyDerivedClassTie(new AMDTieMyDerivedClassI()), Ice.Util.stringToIdentity("test"));
        adapter.activate();
        return WAIT;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);
        //
        // It's possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        //
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.operations.AMD");

        return initData;
    }

    public static void main(String[] args)
    {
        AMDServer c = new AMDServer();
        int status = c.main("AMDServer", args);

        System.gc();
        System.exit(status);
    }
}
