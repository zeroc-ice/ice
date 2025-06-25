# Copyright (c) ZeroC, Inc.

import Ice
import Test
import sys


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def getTCPEndpointInfo(info):
    while info:
        if isinstance(info, Ice.TCPEndpointInfo):
            return info
        info = info.underlying


def getTCPConnectionInfo(info):
    while info:
        if isinstance(info, Ice.TCPConnectionInfo):
            return info
        info = info.underlying


def allTests(helper, communicator):
    sys.stdout.write("testing proxy endpoint information... ")
    sys.stdout.flush()

    p1 = Ice.ObjectPrx(
        communicator,
        "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:"
        + "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:"
        + "opaque -e 1.8 -t 100 -v ABCD",
    )

    endps = p1.ice_getEndpoints()

    endpoint = endps[0].getInfo()
    tcpEndpoint = getTCPEndpointInfo(endpoint)
    test(isinstance(tcpEndpoint, Ice.TCPEndpointInfo))
    test(tcpEndpoint.host == "tcphost")
    test(tcpEndpoint.port == 10000)
    test(tcpEndpoint.sourceAddress == "10.10.10.10")
    test(tcpEndpoint.compress)
    test(not tcpEndpoint.datagram())
    test(
        (tcpEndpoint.type() == Ice.TCPEndpointType and not tcpEndpoint.secure())
        or (tcpEndpoint.type() == Ice.SSLEndpointType and tcpEndpoint.secure())  # SSL
        or (tcpEndpoint.type() == Ice.WSEndpointType and not tcpEndpoint.secure())  # WS
        or (tcpEndpoint.type() == Ice.WSSEndpointType and tcpEndpoint.secure())
    )  # WS
    test(
        (
            tcpEndpoint.type() == Ice.TCPEndpointType
            and isinstance(endpoint, Ice.TCPEndpointInfo)
        )
        or (
            tcpEndpoint.type() == Ice.SSLEndpointType
            and isinstance(endpoint, Ice.SSLEndpointInfo)
        )
        or (
            tcpEndpoint.type() == Ice.WSEndpointType
            and isinstance(endpoint, Ice.WSEndpointInfo)
        )
        or (
            tcpEndpoint.type() == Ice.WSSEndpointType
            and isinstance(endpoint, Ice.WSEndpointInfo)
        )
    )

    udpEndpoint = endps[1].getInfo()
    test(isinstance(udpEndpoint, Ice.UDPEndpointInfo))
    test(udpEndpoint.host == "udphost")
    test(udpEndpoint.port == 10001)
    test(udpEndpoint.sourceAddress == "10.10.10.10")
    test(udpEndpoint.mcastInterface == "eth0")
    test(udpEndpoint.mcastTtl == 5)
    test(not udpEndpoint.compress)
    test(not udpEndpoint.secure())
    test(udpEndpoint.datagram())
    test(udpEndpoint.type() == Ice.UDPEndpointType)

    opaqueEndpoint = endps[2].getInfo()
    test(isinstance(opaqueEndpoint, Ice.OpaqueEndpointInfo))
    test(opaqueEndpoint.rawEncoding == Ice.EncodingVersion(1, 8))

    print("ok")

    sys.stdout.write("test object adapter endpoint information... ")
    sys.stdout.flush()

    host = (
        "::1"
        if communicator.getProperties().getIcePropertyAsInt("Ice.IPv6") != 0
        else "127.0.0.1"
    )
    communicator.getProperties().setProperty(
        "TestAdapter.Endpoints", 'tcp -h "' + host + '" -t 15000:udp -h "' + host + '"'
    )
    adapter = communicator.createObjectAdapter("TestAdapter")
    endpoints = adapter.getEndpoints()
    test(len(endpoints) == 2)
    publishedEndpoints = adapter.getPublishedEndpoints()
    test(endpoints == publishedEndpoints)

    tcpEndpoint = getTCPEndpointInfo(endpoints[0].getInfo())
    test(
        tcpEndpoint.type() == Ice.TCPEndpointType
        or tcpEndpoint.type() == 2
        or tcpEndpoint.type() == 4
        or tcpEndpoint.type() == 5
    )
    test(tcpEndpoint.host == host)
    test(tcpEndpoint.port > 0)

    udpEndpoint = endpoints[1].getInfo()
    test(udpEndpoint.host == host)
    test(udpEndpoint.datagram())
    test(udpEndpoint.port > 0)

    endpoints = (endpoints[0],)
    test(len(endpoints) == 1)
    adapter.setPublishedEndpoints(endpoints)
    publishedEndpoints = adapter.getPublishedEndpoints()
    test(endpoints == publishedEndpoints)

    adapter.destroy()

    communicator.getProperties().setProperty(
        "TestAdapter.Endpoints", "default -h * -p 15000"
    )
    communicator.getProperties().setProperty(
        "TestAdapter.PublishedEndpoints", "default -h 127.0.0.1 -p 15000"
    )
    adapter = communicator.createObjectAdapter("TestAdapter")

    endpoints = adapter.getEndpoints()
    test(len(endpoints) >= 1)
    publishedEndpoints = adapter.getPublishedEndpoints()

    test(len(publishedEndpoints) == 1)

    for i in range(0, len(endpoints)):
        tcpEndpoint = getTCPEndpointInfo(endpoints[i].getInfo())
        test(tcpEndpoint.port == 15000)

    tcpEndpoint = getTCPEndpointInfo(publishedEndpoints[0].getInfo())
    test(tcpEndpoint.host == "127.0.0.1")
    test(tcpEndpoint.port == 15000)

    adapter.destroy()

    print("ok")

    base = communicator.stringToProxy(
        "test:{0}:{1}".format(
            helper.getTestEndpoint(), helper.getTestEndpoint(protocol="udp")
        )
    )
    testIntf = Test.TestIntfPrx.checkedCast(base)

    sys.stdout.write("test connection endpoint information... ")
    sys.stdout.flush()

    defaultHost = communicator.getProperties().getIceProperty("Ice.Default.Host")
    port = helper.getTestPort()

    tcpinfo = getTCPEndpointInfo(base.ice_getConnection().getEndpoint().getInfo())
    test(tcpinfo.port == port)
    test(not tcpinfo.compress)
    test(tcpinfo.host == defaultHost)

    ctx = testIntf.getEndpointInfoAsContext()
    test(ctx["host"] == tcpinfo.host)
    test(ctx["compress"] == "false")
    port = int(ctx["port"])
    test(port > 0)

    udp = base.ice_datagram().ice_getConnection().getEndpoint().getInfo()
    test(udp.port == port)
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
    test(tcpinfo.remotePort == port)
    if defaultHost == "127.0.0.1":
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

    if (
        base.ice_getConnection().type() == "ws"
        or base.ice_getConnection().type() == "wss"
    ):
        test(isinstance(info, Ice.WSConnectionInfo))

        test(info.headers["Upgrade"] == "websocket")
        test(info.headers["Connection"] == "Upgrade")
        test(info.headers["Sec-WebSocket-Protocol"] == "ice.zeroc.com")
        test("Sec-WebSocket-Accept" in info.headers)

        test(ctx["ws.Upgrade"] == "websocket")
        test(ctx["ws.Connection"] == "Upgrade")
        test(ctx["ws.Sec-WebSocket-Protocol"] == "ice.zeroc.com")
        test(ctx["ws.Sec-WebSocket-Version"] == "13")
        test("ws.Sec-WebSocket-Key" in ctx)

    print("ok")

    testIntf.shutdown()

    communicator.shutdown()
    communicator.waitForShutdown()
