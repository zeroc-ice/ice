// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceBox.configuration;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        AllTests.allTests(communicator, getWriter());

        //
        // Shutdown the IceBox server.
        //
        com.zeroc.Ice.ProcessPrx.uncheckedCast(
            communicator.stringToProxy("DemoIceBox/admin -f Process:default -p 9996")).shutdown();
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.IceBox.configuration");
        r.initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
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
