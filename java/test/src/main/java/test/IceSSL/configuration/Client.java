// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceSSL.configuration;

import java.io.PrintWriter;

import test.IceSSL.configuration.Test.ServerFactoryPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        PrintWriter out = getWriter();
        if(args.length < 1)
        {
            out.println("Usage: client testdir");
            return 1;
        }
        ServerFactoryPrx factory = AllTests.allTests(this, args[0]);
        factory.shutdown();
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.IceSSL.configuration");
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
