# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator, collocated):
    sys.stdout.write("testing proxy endpoint information... ")
    sys.stdout.flush()

    p1 = communicator.stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z:" + \
                                    "udp -h udphost -p 10001 --interface eth0 --ttl 5:" + \
                                    "opaque -e 1.8 -t 100 -v ABCD")

    endps = p1.ice_getEndpoints()

    ipEndpoint = endps[0].getInfo()
    test(isinstance(ipEndpoint, Ice.IPEndpointInfo))
    test(ipEndpoint.host == "tcphost")
    test(ipEndpoint.port == 10000)
    test(ipEndpoint.timeout == 1200)
    test(ipEndpoint.compress)
    test(not ipEndpoint.datagram())
    test((ipEndpoint.type() == Ice.TCPEndpointType and not ipEndpoint.secure()) or
         (ipEndpoint.type() == 2 and ipEndpoint.secure())) # SSL
    test((ipEndpoint.type() == Ice.TCPEndpointType and isinstance(ipEndpoint, Ice.TCPEndpointInfo)) or
         (ipEndpoint.type() == 2)) # SSL

    udpEndpoint = endps[1].getInfo()
    test(isinstance(udpEndpoint, Ice.UDPEndpointInfo))
    test(udpEndpoint.host == "udphost")
    test(udpEndpoint.port == 10001)
    test(udpEndpoint.mcastInterface == "eth0")
    test(udpEndpoint.mcastTtl == 5)
    test(udpEndpoint.timeout == -1)
    test(not udpEndpoint.compress)
    test(not udpEndpoint.secure())
    test(udpEndpoint.datagram())
    test(udpEndpoint.type() == Ice.UDPEndpointType)

    opaqueEndpoint = endps[2].getInfo()
    test(isinstance(opaqueEndpoint, Ice.OpaqueEndpointInfo))
    test(opaqueEndpoint.rawEncoding == Ice.EncodingVersion(1, 8))

    print("ok")

    defaultHost = communicator.getProperties().getProperty("Ice.Default.Host")

    sys.stdout.write("test object adapter endpoint information... ")
    sys.stdout.flush()

    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -t 15000:udp")
    adapter = communicator.createObjectAdapter("TestAdapter")
    endpoints = adapter.getEndpoints()
    test(len(endpoints) == 2)
    publishedEndpoints = adapter.getPublishedEndpoints()
    test(endpoints == publishedEndpoints)

    ipEndpoint = endpoints[0].getInfo()
    test(ipEndpoint.type() == Ice.TCPEndpointType or ipEndpoint.type() == 2)
    test(ipEndpoint.host == defaultHost)
    test(ipEndpoint.port > 0)
    test(ipEndpoint.timeout == 15000)

    udpEndpoint = endpoints[1].getInfo()
    test(udpEndpoint.host == defaultHost)
    test(udpEndpoint.datagram())
    test(udpEndpoint.port > 0)

    adapter.destroy()

    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -h * -p 12020")
    communicator.getProperties().setProperty("TestAdapter.PublishedEndpoints", "default -h 127.0.0.1 -p 12020")
    adapter = communicator.createObjectAdapter("TestAdapter")

    endpoints = adapter.getEndpoints()
    test(len(endpoints) >= 1)
    publishedEndpoints = adapter.getPublishedEndpoints()
    test(len(publishedEndpoints) == 1)

    for i in range(0, len(endpoints)):
        ipEndpoint = endpoints[i].getInfo()
        test(ipEndpoint.port == 12020)

    ipEndpoint = publishedEndpoints[0].getInfo()
    test(ipEndpoint.host == "127.0.0.1")
    test(ipEndpoint.port == 12020)

    adapter.destroy()

    print("ok")

    base = communicator.stringToProxy("test:default -p 12010:udp -p 12010")
    testIntf = Test.TestIntfPrx.checkedCast(base)

    sys.stdout.write("test connection endpoint information... ")
    sys.stdout.flush()

    ipinfo = base.ice_getConnection().getEndpoint().getInfo()
    test(ipinfo.port == 12010)
    test(not ipinfo.compress)
    test(ipinfo.host == defaultHost)

    ctx = testIntf.getEndpointInfoAsContext()
    test(ctx["host"] == ipinfo.host)
    test(ctx["compress"] == "false")
    port = int(ctx["port"])
    test(port > 0)

    udp = base.ice_datagram().ice_getConnection().getEndpoint().getInfo()
    test(udp.port == 12010)
    test(udp.host == defaultHost)

    print("ok")

    sys.stdout.write("testing connection information... ")
    sys.stdout.flush()

    info = base.ice_getConnection().getInfo()
    test(not info.incoming)
    test(len(info.adapterName) == 0)
    test(info.remotePort == 12010)
    test(info.remoteAddress == defaultHost)
    test(info.localAddress == defaultHost)

    ctx = testIntf.getConnectionInfoAsContext()
    test(ctx["incoming"] == "true")
    test(ctx["adapterName"] == "TestAdapter")
    test(ctx["remoteAddress"] == info.localAddress)
    test(ctx["localAddress"] == info.remoteAddress)
    test(ctx["remotePort"] == str(info.localPort))
    test(ctx["localPort"] == str(info.remotePort))

    print("ok")

    testIntf.shutdown()

    communicator.shutdown()
    communicator.waitForShutdown()
