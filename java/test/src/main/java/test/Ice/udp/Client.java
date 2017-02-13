// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.udp;

import test.Ice.udp.Test.*;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        AllTests.allTests(this);

        int num;
        try
        {
            num = args.length == 1 ? Integer.parseInt(args[0]) : 1;
        }
        catch(NumberFormatException ex)
        {
            num = 1;
        }
        for(int i = 0; i < num; ++i)
        {
            TestIntfPrx.uncheckedCast(communicator().stringToProxy("control:" + getTestEndpoint(i, "tcp"))).shutdown();
        }
        return 0;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.UDP.RcvSize", "16384");
        initData.properties.setProperty("Ice.UDP.SndSize", "16384");
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
