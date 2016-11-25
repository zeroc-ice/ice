// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
            myClass.opVoid();
            throw new RuntimeException();
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            out.println("ok");
        }

        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        r.initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.operations");
        r.initData.properties.setProperty("Ice.BatchAutoFlushSize", "100");
        return r;
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }
}
