// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        Ice.Communicator communicator = communicator();
        ThrowerPrx thrower = AllTests.allTests(communicator, getWriter());
        thrower.shutdown();
        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.exceptions");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max
        return initData;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
