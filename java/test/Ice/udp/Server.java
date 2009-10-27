// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.udp;

public class Server extends test.Util.Application
{
    public int run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        if(args.length == 1 && args[0].equals("1"))
        {
            properties.setProperty("TestAdapter.Endpoints", "udp -p 12010");
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            adapter.add(new TestIntfI(), communicator().stringToIdentity("test"));
            adapter.activate();
        }

        Ice.ObjectAdapter mcastAdapter = communicator().createObjectAdapter("McastTestAdapter");
        mcastAdapter.add(new TestIntfI(), communicator().stringToIdentity("test"));
        mcastAdapter.activate();

        return WAIT;
    }

    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
	initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.UDP.RcvSize", "16384");
        initData.properties.setProperty("Ice.UDP.SndSize", "16384");

        String host;
        if(initData.properties.getProperty("Ice.IPv6") == "1")
        {
            host = "\"ff01::1:1\"";
        }
        else
        {
            host = "239.255.1.1";
        }
        initData.properties.setProperty("McastTestAdapter.Endpoints", "udp -h " + host + " -p 12020");
        return initData;
    }

    public static void main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);

        System.gc();
        System.exit(status);
    }
}
