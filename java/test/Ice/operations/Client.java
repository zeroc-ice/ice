// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.MyClassPrx;

public class Client extends test.Util.Application
{
    public int run(String[] args)
    {
        java.io.PrintWriter out = getWriter();
        MyClassPrx myClass = AllTests.allTests(this, false, out);

        out.print("testing server shutdown... ");
        out.flush();
        myClass.shutdown();
        try
        {
            myClass.opVoid();
            throw new RuntimeException();
        }
        catch(Ice.LocalException ex)
        {
            out.println("ok");
        }

        return 0;
    }

    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties.setProperty("Ice.ThreadPool.Client.SizeWarn", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.operations");
        //
        // We must set MessageSizeMax to an explicit values,
        // because we run tests to check whether
        // Ice.MemoryLimitException is raised as expected.
        //
        initData.properties.setProperty("Ice.MessageSizeMax", "100");
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
