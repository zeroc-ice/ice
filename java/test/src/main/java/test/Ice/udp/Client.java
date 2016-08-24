// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
            TestIntfPrx.uncheckedCast(communicator().stringToProxy("control:tcp -p " + (12010 + i))).shutdown();
        }
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        r.initData.properties.setProperty("Ice.UDP.RcvSize", "16384");
        r.initData.properties.setProperty("Ice.UDP.SndSize", "16384");
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
