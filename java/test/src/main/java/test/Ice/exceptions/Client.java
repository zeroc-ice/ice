// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

import test.Ice.exceptions.Test.ThrowerPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        ThrowerPrx thrower = AllTests.allTests(communicator, getWriter());
        thrower.shutdown();
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.exceptions");
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        r.initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        return r;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
