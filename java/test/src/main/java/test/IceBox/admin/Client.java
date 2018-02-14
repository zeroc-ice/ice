// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceBox.admin;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        AllTests.allTests(communicator);

        //
        // Shutdown the IceBox server.
        //
        Ice.ProcessPrxHelper.uncheckedCast(
            communicator.stringToProxy("DemoIceBox/admin -f Process:default -p 9996")).shutdown();
        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");
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
