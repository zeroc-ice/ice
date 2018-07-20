// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        java.util.List<String> rargs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, rargs);
        properties.setProperty("Ice.Package.Test", "test.Ice.udp");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.UDP.RcvSize", "16384");
        properties.setProperty("Ice.UDP.SndSize", "16384");

        {
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

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            int num = rargs.size() == 1 ? Integer.parseInt(rargs.get(0)) : 0;

            communicator.getProperties().setProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
            adapter.add(new TestIntfI(), com.zeroc.Ice.Util.stringToIdentity("control"));
            adapter.activate();

            if(num == 0)
            {
                properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
                com.zeroc.Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter");
                adapter2.add(new TestIntfI(), com.zeroc.Ice.Util.stringToIdentity("test"));
                adapter2.activate();
            }

            StringBuilder endpoint = new StringBuilder();
            if(properties.getProperty("Ice.IPv6").equals("1"))
            {
                endpoint.append("udp -h \"ff15::1:1\" -p ");
                endpoint.append(getTestPort(10));
                if(System.getProperty("os.name").contains("OS X") ||
                   System.getProperty("os.name").startsWith("Windows"))
                {
                    endpoint.append(" --interface \"::1\""); // Use loopback to prevent other machines to answer.
                }
            }
            else
            {
                endpoint.append("udp -h 239.255.1.1 -p ");
                endpoint.append(getTestPort(10));
                if(System.getProperty("os.name").contains("OS X") ||
                   System.getProperty("os.name").startsWith("Windows"))
                {
                    endpoint.append(" --interface 127.0.0.1"); // Use loopback to prevent other machines to answer.
                }
            }
            properties.setProperty("McastTestAdapter.Endpoints", endpoint.toString());

            com.zeroc.Ice.ObjectAdapter mcastAdapter = communicator.createObjectAdapter("McastTestAdapter");
            mcastAdapter.add(new TestIntfI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            mcastAdapter.activate();

            serverReady();

            communicator.waitForShutdown();
        }
    }
}
