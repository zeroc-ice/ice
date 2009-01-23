// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        if(args.length == 1 && args[0].equals("1"))
        {
            properties.setProperty("TestAdapter.Endpoints", "udp -p 12010");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestIntfI(), communicator.stringToIdentity("test"));
            adapter.activate();
        }

        String host;
        if(properties.getProperty("Ice.IPv6") == "1")
        {
            host = "\"ff01::1:1\"";
        }
        else
        {
            host = "239.255.1.1";
        }
        properties.setProperty("McastTestAdapter.Endpoints", "udp -h " + host + " -p 12020");
        Ice.ObjectAdapter mcastAdapter = communicator.createObjectAdapter("McastTestAdapter");
        mcastAdapter.add(new TestIntfI(), communicator.stringToIdentity("test"));
        mcastAdapter.activate();

        communicator.waitForShutdown();
        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.StringSeqHolder argHolder = new Ice.StringSeqHolder(args);
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(argHolder);
        
            initData.properties.setProperty("Ice.Warn.Connections", "0");
            initData.properties.setProperty("Ice.UDP.RcvSize", "16384");

            communicator = Ice.Util.initialize(argHolder, initData);
            status = run(argHolder.value, communicator);
        }
        catch(Exception ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
