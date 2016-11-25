// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.retry;

import test.Ice.retry.Test.RetryPrx;

public class Client extends test.Util.Application
{
    private Instrumentation instrumentation = new Instrumentation();

    @Override
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();

        //
        // Configure a second communicator for the invocation timeout
        // + retry test, we need to configure a large retry interval
        // to avoid time-sensitive failures.
        //
        Ice.InitializationData initData2 = createInitializationData();
        initData2.properties = communicator.getProperties()._clone();
        initData2.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
        initData2.observer = instrumentation.getObserver();
        Ice.Communicator communicator2 = initialize(initData2);

        try
        {
            RetryPrx retry = AllTests.allTests(communicator, communicator2, getWriter(), instrumentation,
                                               "retry:" + getTestEndpoint(0));
            retry.shutdown();
            return 0;
        }
        finally
        {
            communicator2.destroy();
        }
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);
        initData.observer = instrumentation.getObserver();

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
