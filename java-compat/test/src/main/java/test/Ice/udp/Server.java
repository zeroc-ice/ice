// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.udp;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.Properties properties = createTestProperties(argsH);
        {
            properties.setProperty("Ice.Package.Test", "test.Ice.udp");
            properties.setProperty("Ice.Warn.Connections", "0");
            properties.setProperty("Ice.UDP.RcvSize", "16384");
            properties.setProperty("Ice.UDP.SndSize", "16384");

            String endpoint;
            if(properties.getProperty("Ice.IPv6").equals("1"))
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
            properties.setProperty("McastTestAdapter.Endpoints", endpoint);
        }

        int port = 0;
        try
        {
            port += argsH.value.length == 1 ? Integer.parseInt(argsH.value[0]) : 0;
        }
        catch(NumberFormatException ex)
        {
        }

        try(Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("ControlAdapter.Endpoints", getTestEndpoint(port, "tcp"));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
            adapter.add(new TestIntfI(), Ice.Util.stringToIdentity("control"));
            adapter.activate();

            if(port == 0)
            {
                properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(port, "udp"));
                Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter");
                adapter2.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
                adapter2.activate();
            }

            StringBuilder endpoint = new StringBuilder();
            if(properties.getProperty("Ice.IPv6").equals("1"))
            {
                endpoint.append("udp -h \"ff15::1:1\" -p ");
                endpoint.append(getTestPort(properties, 10));
                if(System.getProperty("os.name").contains("OS X") ||
                   System.getProperty("os.name").startsWith("Windows"))
                {
                    endpoint.append(" --interface \"::1\"");
                }
            }
            else
            {
                endpoint.append("udp -h 239.255.1.1 -p ");
                endpoint.append(getTestPort(properties, 10));
                if(System.getProperty("os.name").contains("OS X") ||
                   System.getProperty("os.name").startsWith("Windows"))
                {
                    endpoint.append(" --interface 127.0.0.1");
                }
            }
            properties.setProperty("McastTestAdapter.Endpoints", endpoint.toString());
            Ice.ObjectAdapter mcastAdapter = communicator.createObjectAdapter("McastTestAdapter");
            mcastAdapter.add(new TestIntfI(), Ice.Util.stringToIdentity("test"));
            mcastAdapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
