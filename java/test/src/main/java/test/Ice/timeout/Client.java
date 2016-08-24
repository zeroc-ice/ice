// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;

import test.Ice.timeout.Test.TimeoutPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        TimeoutPrx timeout = AllTests.allTests(this);
        timeout.shutdown();
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.timeout");

        //
        // We need to send messages large enough to cause the transport
        // buffers to fill up.
        //
        r.initData.properties.setProperty("Ice.MessageSizeMax", "20000");

        //
        // For this test, we want to disable retries.
        //
        r.initData.properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");

        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        r.initData.properties.setProperty("Ice.TCP.SndSize", "50000");

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
