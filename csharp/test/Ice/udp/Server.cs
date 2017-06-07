// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Text;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestCommon.Application
{
    public override int run(string[] args)
    {
        Ice.Properties properties = communicator().getProperties();

        int num = 0;
        try
        {
            num = args.Length == 1 ? Int32.Parse(args[0]) : 0;
        }
        catch(FormatException)
        {
        }
        properties.setProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("ControlAdapter");
        adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("control"));
        adapter.activate();

        if(num == 0)
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
            Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("TestAdapter");
            adapter2.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
            adapter2.activate();
        }

        StringBuilder endpoint = new StringBuilder();
        if(properties.getProperty("Ice.IPv6").Equals("1"))
        {
            endpoint.Append("udp -h \"ff15::1:1\" --interface \"::1\" -p "); // Use loopback to prevent other machines to answer.
        }
        else
        {
            endpoint.Append("udp -h 239.255.1.1 --interface 127.0.0.1 -p "); // Use loopback to prevent other machines to answer.
        }
        endpoint.Append(getTestPort(properties, 10));
        properties.setProperty("McastTestAdapter.Endpoints", endpoint.ToString());
        Ice.ObjectAdapter mcastAdapter = communicator().createObjectAdapter("McastTestAdapter");
        mcastAdapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
        mcastAdapter.activate();

        communicator().waitForShutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.UDP.RcvSize", "16384");
        return initData;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
