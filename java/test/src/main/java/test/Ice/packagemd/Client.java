// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.packagemd;

import test.Ice.packagemd.Test.InitialPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        InitialPrx initial = AllTests.allTests(communicator(), getWriter());
        initial.shutdown();
        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.packagemd");
        initData.properties.setProperty("Ice.Package.Test1", "test.Ice.packagemd");
        return initData;
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }
}
