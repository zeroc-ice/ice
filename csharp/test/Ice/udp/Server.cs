// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server
{
    private static int run(string[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();

        int port = 12010;
        try
        {
            port += args.Length == 1 ? System.Int32.Parse(args[0]) : 0;
        }
        catch(System.FormatException)
        {
        }
        properties.setProperty("ControlAdapter.Endpoints", "tcp -p " + port);
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
        adapter.add(new TestIntfI(), communicator.stringToIdentity("control"));
        adapter.activate();

        if(port == 12010)
        {
            properties.setProperty("TestAdapter.Endpoints", "udp -p 12010");
            Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter");
            adapter2.add(new TestIntfI(), communicator.stringToIdentity("test"));
            adapter2.activate();
        }

        string endpoint;
        if(properties.getProperty("Ice.IPv6").Equals("1"))
        {
            if(IceInternal.AssemblyUtil.osx_)
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
        properties.setProperty("McastTestAdapter.Endpoints", endpoint);
        Ice.ObjectAdapter mcastAdapter = communicator.createObjectAdapter("McastTestAdapter");
        mcastAdapter.add(new TestIntfI(), communicator.stringToIdentity("test"));
        mcastAdapter.activate();

        communicator.waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = Ice.Util.createProperties(ref args);

            initData.properties.setProperty("Ice.Warn.Connections", "0");
            initData.properties.setProperty("Ice.UDP.RcvSize", "16384");

            communicator = Ice.Util.initialize(ref args, initData);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            System.Console.Error.WriteLine(ex);
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
                System.Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        return status;
    }
}
