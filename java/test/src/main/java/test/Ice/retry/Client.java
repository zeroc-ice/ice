// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.retry;

import test.Ice.retry.Test.RetryPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        RetryPrx retry = AllTests.allTests(communicator, getWriter());
        retry.shutdown();
        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        initData.observer = Instrumentation.getObserver();

        initData.properties.setProperty("Ice.Package.Test", "test.Ice.retry");

        initData.properties.setProperty("Ice.RetryIntervals", "0 1 400 1");

        //
        // We don't want connection warnings because of the timeout
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");

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
