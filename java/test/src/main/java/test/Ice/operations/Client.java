// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.MyClassPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        java.io.PrintWriter out = getWriter();
        MyClassPrx myClass = AllTests.allTests(this);

        out.print("testing server shutdown... ");
        out.flush();
        myClass.shutdown();
        try
        {
            myClass.ice_timeout(100).ice_ping(); // Use timeout to speed up testing on Windows
            throw new RuntimeException();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            out.println("ok");
        }

        return 0;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.operations");
        initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
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
