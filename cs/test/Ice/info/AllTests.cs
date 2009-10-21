// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using Test;

public class AllTests
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.Exception();
        }
    }


    public static void allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing proxy endpoint information... ");
        Console.Out.Flush();
        {
            Ice.ObjectPrx p1 = communicator.stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z:" +
                                                          "udp -h udphost -p 10001 --interface eth0 --ttl 5:" +
                                                          "opaque -t 100 -v ABCD");

            Ice.Endpoint[] endps = p1.ice_getEndpoints();


            Ice.IPEndpointInfo ipEndpoint = (Ice.IPEndpointInfo)endps[0].getInfo();
            test(ipEndpoint.host.Equals("tcphost"));
            test(ipEndpoint.port == 10000);
            test(ipEndpoint.timeout == 1200);
            test(ipEndpoint.compress);
            test(!ipEndpoint.datagram());

#if __MonoCS__
            test(ipEndpoint.type() == Ice.TCPEndpointType.value && !ipEndpoint.secure());
            test(ipEndpoint.type() == Ice.TCPEndpointType.value && ipEndpoint is Ice.TCPEndpointInfo);
#else
            test(ipEndpoint.type() == Ice.TCPEndpointType.value && !ipEndpoint.secure() ||
                 ipEndpoint.type() == IceSSL.EndpointType.value && ipEndpoint.secure());
            test(ipEndpoint.type() == Ice.TCPEndpointType.value && ipEndpoint is Ice.TCPEndpointInfo ||
                 ipEndpoint.type() == IceSSL.EndpointType.value && ipEndpoint is IceSSL.EndpointInfo);
#endif

            Ice.UDPEndpointInfo udpEndpoint = (Ice.UDPEndpointInfo)endps[1].getInfo();
            test(udpEndpoint.host.Equals("udphost"));
            test(udpEndpoint.port == 10001);
            test(udpEndpoint.mcastInterface.Equals("eth0"));
            test(udpEndpoint.mcastTtl == 5);
            test(udpEndpoint.timeout == -1);
            test(!udpEndpoint.compress);
            test(!udpEndpoint.secure());
            test(udpEndpoint.datagram());
            test(udpEndpoint.type() == 3);

            Ice.OpaqueEndpointInfo opaqueEndpoint = (Ice.OpaqueEndpointInfo)endps[2].getInfo();
            test(opaqueEndpoint.rawBytes.Length > 0);
        }
        Console.Out.WriteLine("ok");

        string defaultHost = communicator.getProperties().getProperty("Ice.Default.Host");
        Ice.ObjectAdapter adapter;

        Console.Out.Write("test object adapter endpoint information... ");
        Console.Out.Flush();
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -t 15000:udp");
            adapter = communicator.createObjectAdapter("TestAdapter");

            Ice.Endpoint[] endpoints = adapter.getEndpoints();
            test(endpoints.Length == 2);
            Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
            test(IceUtilInternal.Arrays.Equals(endpoints, publishedEndpoints));

            Ice.IPEndpointInfo ipEndpoint = (Ice.IPEndpointInfo)endpoints[0].getInfo();
#if __MonoCS__
            test(ipEndpoint.type() == Ice.TCPEndpointType.value);
#else
            test(ipEndpoint.type() == Ice.TCPEndpointType.value || ipEndpoint.type() == IceSSL.EndpointType.value);
#endif
            test(ipEndpoint.host.Equals(defaultHost));
            test(ipEndpoint.port > 0);
            test(ipEndpoint.timeout == 15000);

            Ice.UDPEndpointInfo udpEndpoint = (Ice.UDPEndpointInfo)endpoints[1].getInfo();
            test(udpEndpoint.host.Equals(defaultHost));
            test(udpEndpoint.datagram());
            test(udpEndpoint.port > 0);

            adapter.destroy();

            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -h * -p 12020");
            communicator.getProperties().setProperty("TestAdapter.PublishedEndpoints", "default -h 127.0.0.1 -p 12020");
            adapter = communicator.createObjectAdapter("TestAdapter");

            endpoints = adapter.getEndpoints();
            test(endpoints.Length >= 1);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(publishedEndpoints.Length == 1);

            foreach(Ice.Endpoint endpoint in endpoints)
            {
                ipEndpoint = (Ice.IPEndpointInfo)endpoint.getInfo();
                test(ipEndpoint.port == 12020);
            }

            ipEndpoint = (Ice.IPEndpointInfo)publishedEndpoints[0].getInfo();
            test(ipEndpoint.host.Equals("127.0.0.1"));
            test(ipEndpoint.port == 12020);

            adapter.destroy();
        }
        Console.Out.WriteLine("ok");

        Ice.ObjectPrx @base = communicator.stringToProxy("test:default -p 12010:udp -p 12010");
        TestIntfPrx testIntf = TestIntfPrxHelper.checkedCast(@base);

        Console.Out.Write("test connection endpoint information... ");
        Console.Out.Flush();
        {
            Ice.EndpointInfo info = @base.ice_getConnection().getEndpoint().getInfo();
            Ice.IPEndpointInfo ipinfo = (Ice.IPEndpointInfo)info;
            test(ipinfo.port == 12010);
            test(!ipinfo.compress);
            test(ipinfo.host.Equals(defaultHost));

            Dictionary<string, string> ctx = testIntf.getEndpointInfoAsContext();
            test(ctx["host"].Equals(ipinfo.host));
            test(ctx["compress"].Equals("false"));
            int port = System.Int32.Parse(ctx["port"]);
            test(port > 0);

            info = @base.ice_datagram().ice_getConnection().getEndpoint().getInfo();
            Ice.UDPEndpointInfo udp = (Ice.UDPEndpointInfo)info;
            test(udp.port == 12010);
            test(udp.host.Equals(defaultHost));
        }
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing connection information... ");
        Console.Out.Flush();
        {
            Ice.IPConnectionInfo info = (Ice.IPConnectionInfo)@base.ice_getConnection().getInfo();
            test(!info.incoming);
            test(info.adapterName.Length == 0);
            test(info.remotePort == 12010);
            test(info.remoteAddress.Equals(defaultHost));
            test(info.localAddress.Equals(defaultHost));

            Dictionary<string, string> ctx = testIntf.getConnectionInfoAsContext();
            test(ctx["incoming"].Equals("true"));
            test(ctx["adapterName"].Equals("TestAdapter"));
            test(ctx["remoteAddress"].Equals(info.localAddress));
            test(ctx["localAddress"].Equals(info.remoteAddress));
            test(ctx["remotePort"].Equals(info.localPort.ToString()));
            test(ctx["localPort"].Equals(info.remotePort.ToString()));
        }
        Console.Out.WriteLine("ok");

        testIntf.shutdown();

        communicator.shutdown();
        communicator.waitForShutdown();
    }
}
