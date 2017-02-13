# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def allTests(communicator):
    sys.stdout.write("testing proxy endpoint information... ")
    sys.stdout.flush()

    p1 = communicator.stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" + \
                                    "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" + \
                                    "opaque -e 1.8 -t 100 -v ABCD")

    endps = p1.ice_getEndpoints()

    ipEndpoint = endps[0].getInfo()
    test(isinstance(ipEndpoint, Ice.IPEndpointInfo))
    test(ipEndpoint.host == "tcphost")
    test(ipEndpoint.port == 10000)
    test(ipEndpoint.sourceAddress == "10.10.10.10")
    test(ipEndpoint.timeout == 1200)
    test(ipEndpoint.compress)
    test(not ipEndpoint.datagram())
    test((ipEndpoint.type() == Ice.TCPEndpointType and not ipEndpoint.secure()) or
         (ipEndpoint.type() == Ice.SSLEndpointType and ipEndpoint.secure()) or # SSL
         (ipEndpoint.type() == Ice.WSEndpointType and not ipEndpoint.secure()) or # WS
         (ipEndpoint.type() == Ice.WSSEndpointType and ipEndpoint.secure())) # WS
    test((ipEndpoint.type() == Ice.TCPEndpointType and isinstance(ipEndpoint, Ice.TCPEndpointInfo)) or
         (ipEndpoint.type() == Ice.SSLEndpointType and isinstance(ipEndpoint, Ice.SSLEndpointInfo)) or
         (ipEndpoint.type() == Ice.WSEndpointType and isinstance(ipEndpoint, Ice.WSEndpointInfo)) or
         (ipEndpoint.type() == Ice.WSSEndpointType and isinstance(ipEndpoint, Ice.WSSEndpointInfo)))

    udpEndpoint = endps[1].getInfo()
    test(isinstance(udpEndpoint, Ice.UDPEndpointInfo))
    test(udpEndpoint.host == "udphost")
    test(udpEndpoint.port == 10001)
    test(udpEndpoint.sourceAddress == "10.10.10.10")
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
    test(ipEndpoint.type() == Ice.TCPEndpointType or ipEndpoint.type() == 2 or ipEndpoint.type() == 4 or
         ipEndpoint.type() == 5)
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

    connection = base.ice_getConnection()
    connection.setBufferSize(1024, 2048)

    info = connection.getInfo()
    test(not info.incoming)
    test(len(info.adapterName) == 0)
    test(info.remotePort == 12010)
    if defaultHost == '127.0.0.1':
        test(info.remoteAddress == defaultHost)
        test(info.localAddress == defaultHost)
    test(info.rcvSize >= 1024)
    test(info.sndSize >= 2048)

    ctx = testIntf.getConnectionInfoAsContext()
    test(ctx["incoming"] == "true")
    test(ctx["adapterName"] == "TestAdapter")
    test(ctx["remoteAddress"] == info.localAddress)
    test(ctx["localAddress"] == info.remoteAddress)
    test(ctx["remotePort"] == str(info.localPort))
    test(ctx["localPort"] == str(info.remotePort))

    if(base.ice_getConnection().type() == "ws" or base.ice_getConnection().type() == "wss"):
        test((base.ice_getConnection().type() == "ws" and isinstance(info, Ice.WSConnectionInfo)) or
             (base.ice_getConnection().type() == "wss" and isinstance(info, Ice.WSSConnectionInfo)))

        test(info.headers["Upgrade"] == "websocket")
        test(info.headers["Connection"] == "Upgrade")
        test(info.headers["Sec-WebSocket-Protocol"] == "ice.zeroc.com")
        test("Sec-WebSocket-Accept" in info.headers);

        test(ctx["ws.Upgrade"] == "websocket")
        test(ctx["ws.Connection"] == "Upgrade")
        test(ctx["ws.Sec-WebSocket-Protocol"] == "ice.zeroc.com")
        test(ctx["ws.Sec-WebSocket-Version"] == "13")
        test("ws.Sec-WebSocket-Key" in ctx)

    print("ok")

    testIntf.shutdown()

    communicator.shutdown()
    communicator.waitForShutdown()
