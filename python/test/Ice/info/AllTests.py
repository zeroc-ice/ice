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

def getTCPEndpointInfo(info):
    while(info):
        if isinstance(info, Ice.TCPEndpointInfo):
            return info
        info = info.underlying

def getTCPConnectionInfo(info):
    while(info):
        if isinstance(info, Ice.TCPConnectionInfo):
            return info
        info = info.underlying

def allTests(communicator):
    sys.stdout.write("testing proxy endpoint information... ")
    sys.stdout.flush()

    p1 = communicator.stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" + \
                                    "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" + \
                                    "opaque -e 1.8 -t 100 -v ABCD")

    endps = p1.ice_getEndpoints()

    endpoint = endps[0].getInfo()
    tcpEndpoint = getTCPEndpointInfo(endpoint)
    test(isinstance(tcpEndpoint, Ice.TCPEndpointInfo))
    test(tcpEndpoint.host == "tcphost")
    test(tcpEndpoint.port == 10000)
    test(tcpEndpoint.sourceAddress == "10.10.10.10")
    test(tcpEndpoint.timeout == 1200)
    test(tcpEndpoint.compress)
    test(not tcpEndpoint.datagram())
    test((tcpEndpoint.type() == Ice.TCPEndpointType and not tcpEndpoint.secure()) or
         (tcpEndpoint.type() == Ice.SSLEndpointType and tcpEndpoint.secure()) or # SSL
         (tcpEndpoint.type() == Ice.WSEndpointType and not tcpEndpoint.secure()) or # WS
         (tcpEndpoint.type() == Ice.WSSEndpointType and tcpEndpoint.secure())) # WS
    test((tcpEndpoint.type() == Ice.TCPEndpointType and isinstance(endpoint, Ice.TCPEndpointInfo)) or
         (tcpEndpoint.type() == Ice.SSLEndpointType and isinstance(endpoint, Ice.SSLEndpointInfo)) or
         (tcpEndpoint.type() == Ice.WSEndpointType and isinstance(endpoint, Ice.WSEndpointInfo)) or
         (tcpEndpoint.type() == Ice.WSSEndpointType and isinstance(endpoint, Ice.WSEndpointInfo)))

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

    tcpEndpoint = getTCPEndpointInfo(endpoints[0].getInfo())
    test(tcpEndpoint.type() == Ice.TCPEndpointType or tcpEndpoint.type() == 2 or tcpEndpoint.type() == 4 or
         tcpEndpoint.type() == 5)
    test(tcpEndpoint.host == defaultHost)
    test(tcpEndpoint.port > 0)
    test(tcpEndpoint.timeout == 15000)

    udpEndpoint = endpoints[1].getInfo()
    test(udpEndpoint.host == defaultHost)
    test(udpEndpoint.datagram())
    test(udpEndpoint.port > 0)

    endpoints = (endpoints[0], )
    test(len(endpoints) == 1)
    adapter.setPublishedEndpoints(endpoints)
    publishedEndpoints = adapter.getPublishedEndpoints()
    test(endpoints == publishedEndpoints)

    adapter.destroy()

    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -h * -p 12020")
    communicator.getProperties().setProperty("TestAdapter.PublishedEndpoints", "default -h 127.0.0.1 -p 12020")
    adapter = communicator.createObjectAdapter("TestAdapter")

    endpoints = adapter.getEndpoints()
    test(len(endpoints) >= 1)
    publishedEndpoints = adapter.getPublishedEndpoints()

    test(len(publishedEndpoints) == 1)

    for i in range(0, len(endpoints)):
        tcpEndpoint = getTCPEndpointInfo(endpoints[i].getInfo())
        test(tcpEndpoint.port == 12020)

    tcpEndpoint = getTCPEndpointInfo(publishedEndpoints[0].getInfo())
    test(tcpEndpoint.host == "127.0.0.1")
    test(tcpEndpoint.port == 12020)

    adapter.destroy()

    print("ok")

    base = communicator.stringToProxy("test:default -p 12010:udp -p 12010")
    testIntf = Test.TestIntfPrx.checkedCast(base)

    sys.stdout.write("test connection endpoint information... ")
    sys.stdout.flush()

    tcpinfo = getTCPEndpointInfo(base.ice_getConnection().getEndpoint().getInfo())
    test(tcpinfo.port == 12010)
    test(not tcpinfo.compress)
    test(tcpinfo.host == defaultHost)

    ctx = testIntf.getEndpointInfoAsContext()
    test(ctx["host"] == tcpinfo.host)
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
    tcpinfo = getTCPConnectionInfo(info)
    test(not info.incoming)
    test(len(info.adapterName) == 0)
    test(tcpinfo.remotePort == 12010)
    if defaultHost == '127.0.0.1':
        test(tcpinfo.remoteAddress == defaultHost)
        test(tcpinfo.localAddress == defaultHost)
    test(tcpinfo.rcvSize >= 1024)
    test(tcpinfo.sndSize >= 2048)

    ctx = testIntf.getConnectionInfoAsContext()
    test(ctx["incoming"] == "true")
    test(ctx["adapterName"] == "TestAdapter")
    test(ctx["remoteAddress"] == tcpinfo.localAddress)
    test(ctx["localAddress"] == tcpinfo.remoteAddress)
    test(ctx["remotePort"] == str(tcpinfo.localPort))
    test(ctx["localPort"] == str(tcpinfo.remotePort))

    if(base.ice_getConnection().type() == "ws" or base.ice_getConnection().type() == "wss"):
        test(isinstance(info, Ice.WSConnectionInfo))

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
