// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.metrics;

import test.Ice.metrics.Test.MetricsPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        try
        {
            MetricsPrx metrics = AllTests.allTests(this, _observer);
            metrics.shutdown();
        }
        catch(com.zeroc.Ice.UserException ex)
        {
            ex.printStackTrace();
            assert(false);
            return 1;
        }
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.metrics");
        r.initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        r.initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        r.initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        r.initData.properties.setProperty("Ice.MessageSizeMax", "50000");
        r.initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");

        r.initData.observer = _observer;
        return r;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }

    private CommunicatorObserverI _observer = new CommunicatorObserverI();
}
