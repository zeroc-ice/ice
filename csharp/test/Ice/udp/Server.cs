// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.RcvSize", "16384");

        using(var communicator = initialize(properties))
        {
            int num = 0;
            try
            {
                num = args.Length == 1 ? Int32.Parse(args[0]) : 0;
            }
            catch(FormatException)
            {
            }

            communicator.getProperties().setProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
            adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("control"));
            adapter.activate();

            if(num == 0)
            {
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
                Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter");
                adapter2.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
                adapter2.activate();
            }

            StringBuilder endpoint = new StringBuilder();
            //
            // Use loopback to prevent other machines to answer.
            //
            if(properties.getProperty("Ice.IPv6").Equals("1"))
            {
                endpoint.Append("udp -h \"ff15::1:1\" --interface \"::1\" -p ");
            }
            else
            {
                endpoint.Append("udp -h 239.255.1.1 --interface 127.0.0.1 -p ");
            }
            endpoint.Append(getTestPort(properties, 10));
            communicator.getProperties().setProperty("McastTestAdapter.Endpoints", endpoint.ToString());
            Ice.ObjectAdapter mcastAdapter = communicator.createObjectAdapter("McastTestAdapter");
            mcastAdapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
            mcastAdapter.activate();

            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
