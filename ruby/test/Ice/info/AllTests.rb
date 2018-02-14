# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def allTests(communicator)
    print "testing proxy endpoint information..."
    STDOUT.flush

    p1 = communicator.stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" + \
                                    "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" + \
                                    "opaque -e 1.8 -t 100 -v ABCD")

    endps = p1.ice_getEndpoints()

    ipEndpoint = endps[0].getInfo()
    test(ipEndpoint.is_a?(Ice::IPEndpointInfo));
    test(ipEndpoint.host == "tcphost")
    test(ipEndpoint.port == 10000)
    test(ipEndpoint.sourceAddress == "10.10.10.10")
    test(ipEndpoint.timeout == 1200)
    test(ipEndpoint.compress)
    test(!ipEndpoint.datagram())
    test((ipEndpoint.type() == Ice::TCPEndpointType && !ipEndpoint.secure()) ||
         (ipEndpoint.type() == Ice::SSLEndpointType && ipEndpoint.secure()) ||
         (ipEndpoint.type() == Ice::WSEndpointType && !ipEndpoint.secure()) ||
         (ipEndpoint.type() == Ice::WSSEndpointType && ipEndpoint.secure()))
    test((ipEndpoint.type() == Ice::TCPEndpointType && ipEndpoint.is_a?(Ice::TCPEndpointInfo)) ||
         (ipEndpoint.type() == Ice::SSLEndpointType && ipEndpoint.is_a?(Ice::SSLEndpointInfo)) ||
         (ipEndpoint.type() == Ice::WSEndpointType && ipEndpoint.is_a?(Ice::WSEndpointInfo)) ||
         (ipEndpoint.type() == Ice::WSSEndpointType && ipEndpoint.is_a?(Ice::WSSEndpointInfo)))

    udpEndpoint = endps[1].getInfo()
    test(udpEndpoint.is_a?(Ice::UDPEndpointInfo));
    test(udpEndpoint.host == "udphost")
    test(udpEndpoint.port == 10001)
    test(udpEndpoint.sourceAddress == "10.10.10.10")
    test(udpEndpoint.mcastInterface == "eth0")
    test(udpEndpoint.mcastTtl == 5)
    test(udpEndpoint.timeout == -1)
    test(!udpEndpoint.compress)
    test(!udpEndpoint.secure())
    test(udpEndpoint.datagram())
    test(udpEndpoint.type() == Ice::UDPEndpointType)

    opaqueEndpoint = endps[2].getInfo()
    test(opaqueEndpoint.is_a?(Ice::OpaqueEndpointInfo));
    test(opaqueEndpoint.rawEncoding == Ice::EncodingVersion.new(1, 8))

    puts "ok"

    defaultHost = communicator.getProperties().getProperty("Ice.Default.Host")
    base = communicator.stringToProxy("test:default -p 12010:udp -p 12010")
    testIntf = Test::TestIntfPrx::checkedCast(base)

    print "test connection endpoint information..."
    STDOUT.flush

    ipinfo = base.ice_getConnection().getEndpoint().getInfo()
    test(ipinfo.port == 12010)
    test(!ipinfo.compress)
    test(ipinfo.host == defaultHost)

    ctx = testIntf.getEndpointInfoAsContext()
    test(ctx["host"] == ipinfo.host)
    test(ctx["compress"] == "false")
    port = Integer(ctx["port"])
    test(port > 0)

    udp = base.ice_datagram().ice_getConnection().getEndpoint().getInfo()
    test(udp.port == 12010)
    test(udp.host == defaultHost)

    puts "ok"

    print "testing connection information..."
    STDOUT.flush

    connection = base.ice_getConnection()
    connection.setBufferSize(1024, 2048)

    info = connection.getInfo()
    test(!info.incoming)
    test(info.adapterName.length == 0)
    test(info.remotePort == 12010)
    if defaultHost == "127.0.0.1"
        test(info.remoteAddress == defaultHost)
        test(info.localAddress == defaultHost)
    end
    test(info.rcvSize >= 1024)
    test(info.sndSize >= 2048)

    ctx = testIntf.getConnectionInfoAsContext()
    test(ctx["incoming"] == "true")
    test(ctx["adapterName"] == "TestAdapter")
    test(ctx["remoteAddress"] == info.localAddress)
    test(ctx["localAddress"] == info.remoteAddress)
    test(ctx["remotePort"] == info.localPort.to_s())
    test(ctx["localPort"] == info.remotePort.to_s())

    if base.ice_getConnection().type() == "ws" || base.ice_getConnection().type() == "wss"
        test((base.ice_getConnection().type() == "ws" && info.is_a?(Ice::WSConnectionInfo)) ||
             (base.ice_getConnection().type() == "wss" && info.is_a?(Ice::WSSConnectionInfo)))

        test(info.headers["Upgrade"] == "websocket")
        test(info.headers["Connection"] == "Upgrade")
        test(info.headers["Sec-WebSocket-Protocol"] == "ice.zeroc.com")
        test(info.headers.has_key?("Sec-WebSocket-Accept"))

        test(ctx["ws.Upgrade"] == "websocket")
        test(ctx["ws.Connection"] == "Upgrade")
        test(ctx["ws.Sec-WebSocket-Protocol"] == "ice.zeroc.com")
        test(ctx["ws.Sec-WebSocket-Version"] == "13")
        test(ctx.has_key?("ws.Sec-WebSocket-Key"))
    end

    puts "ok"

    testIntf.shutdown()

    communicator.shutdown()
end
