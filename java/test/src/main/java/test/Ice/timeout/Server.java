// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.timeout;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new TimeoutI(), com.zeroc.Ice.Util.stringToIdentity("timeout"));
        adapter.activate();
        return WAIT;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.timeout");
        r.initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        r.initData.properties.setProperty("Ice.TCP.RcvSize", "50000");

        //
        // This test kills connections, so we don't want warnings.
        //
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        return r;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
