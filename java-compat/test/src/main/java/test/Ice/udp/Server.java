// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.udp;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();

        int port = 0;
        try
        {
            port += args.length == 1 ? Integer.parseInt(args[0]) : 0;
        }
        catch(NumberFormatException ex)
        {
        }
        properties.setProperty("ControlAdapter.Endpoints", getTestEndpoint(port, "tcp"));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("ControlAdapter");
        adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("control"));
        adapter.activate();

        if(port == 0)
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(port, "udp"));
            Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("TestAdapter");
            adapter2.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
            adapter2.activate();
        }

        if(!isAndroid())
        {
            Ice.ObjectAdapter mcastAdapter = communicator().createObjectAdapter("McastTestAdapter");
            mcastAdapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
            mcastAdapter.activate();
        }

        return WAIT;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = super.getInitData(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.UDP.RcvSize", "16384");
        initData.properties.setProperty("Ice.UDP.SndSize", "16384");

        if(!isAndroid())
        {
            String endpoint;
            if(initData.properties.getProperty("Ice.IPv6").equals("1"))
            {
                if(System.getProperty("os.name").contains("OS X"))
                {
                    endpoint = "udp -h \"ff15::1:1\" -p 12020 --interface \"::1\"";
                }
                else
                {
                    endpoint = "udp -h \"ff15::1:1\" -p 12020";
                }
            }
            else
            {
                endpoint = "udp -h 239.255.1.1 -p 12020";
            }
            initData.properties.setProperty("McastTestAdapter.Endpoints", endpoint);
        }
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
