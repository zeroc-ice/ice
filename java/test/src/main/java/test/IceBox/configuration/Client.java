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
        AllTests.allTests(this);

        //
        // Shutdown the IceBox server.
        //
        com.zeroc.Ice.ProcessPrx.uncheckedCast(
            communicator().stringToProxy("DemoIceBox/admin -f Process:default -p 9996")).shutdown();
        return 0;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.IceBox.configuration");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
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
