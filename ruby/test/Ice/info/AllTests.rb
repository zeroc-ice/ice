#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

def getTCPEndpointInfo(info)
    while info
        if info.is_a?(Ice::TCPEndpointInfo)
            return info
        end
        info = info.underlying
    end
end

def getTCPConnectionInfo(info)
    while info
        if info.is_a?(Ice::TCPConnectionInfo)
            return info
        end
        info = info.underlying
    end
end

def allTests(helper, communicator)
    print "testing proxy endpoint information..."
    STDOUT.flush

    p1 = communicator.stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" + \
                                    "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" + \
                                    "opaque -e 1.8 -t 100 -v ABCD")

    endps = p1.ice_getEndpoints()
    endpoint = endps[0].getInfo()
    tcpEndpoint = getTCPEndpointInfo(endpoint)
    test(tcpEndpoint.is_a?(Ice::TCPEndpointInfo));
    test(tcpEndpoint.host == "tcphost")
    test(tcpEndpoint.port == 10000)
    test(tcpEndpoint.sourceAddress == "10.10.10.10")
    test(tcpEndpoint.timeout == 1200)
    test(tcpEndpoint.compress)
    test(!tcpEndpoint.datagram())
    test((tcpEndpoint.type() == Ice::TCPEndpointType && !tcpEndpoint.secure()) ||
         (tcpEndpoint.type() == Ice::SSLEndpointType && tcpEndpoint.secure()) ||
         (tcpEndpoint.type() == Ice::WSEndpointType && !tcpEndpoint.secure()) ||
         (tcpEndpoint.type() == Ice::WSSEndpointType && tcpEndpoint.secure()))
    test((tcpEndpoint.type() == Ice::TCPEndpointType && endpoint.is_a?(Ice::TCPEndpointInfo)) ||
         (tcpEndpoint.type() == Ice::SSLEndpointType && endpoint.is_a?(Ice::SSLEndpointInfo)) ||
         (tcpEndpoint.type() == Ice::WSEndpointType && endpoint.is_a?(Ice::WSEndpointInfo)) ||
         (tcpEndpoint.type() == Ice::WSSEndpointType && endpoint.is_a?(Ice::WSEndpointInfo)))

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
    tcpEndpoint = helper.getTestEndpoint()
    udpEndpoint = helper.getTestEndpoint(protocol:"udp")
    base = communicator.stringToProxy("test:#{tcpEndpoint}:#{udpEndpoint}")
    testIntf = Test::TestIntfPrx::checkedCast(base)

    print "test connection endpoint information..."
    STDOUT.flush
    port = helper.getTestPort()
    tcpinfo = getTCPEndpointInfo(base.ice_getConnection().getEndpoint().getInfo())
    test(tcpinfo.port == port)
    test(!tcpinfo.compress)
    test(tcpinfo.host == defaultHost)

    ctx = testIntf.getEndpointInfoAsContext()
    test(ctx["host"] == tcpinfo.host)
    test(ctx["compress"] == "false")
    port = Integer(ctx["port"])
    test(port > 0)

    udp = base.ice_datagram().ice_getConnection().getEndpoint().getInfo()
    test(udp.port == port)
    test(udp.host == defaultHost)

    puts "ok"

    print "testing connection information..."
    STDOUT.flush

    connection = base.ice_getConnection()
    connection.setBufferSize(1024, 2048)

    info = connection.getInfo()
    tcpinfo = getTCPConnectionInfo(info)

    test(!info.incoming)
    test(info.adapterName.length == 0)
    test(tcpinfo.remotePort == port)
    if defaultHost == "127.0.0.1"
        test(tcpinfo.remoteAddress == defaultHost)
        test(tcpinfo.localAddress == defaultHost)
    end
    test(tcpinfo.rcvSize >= 1024)
    test(tcpinfo.sndSize >= 2048)

    ctx = testIntf.getConnectionInfoAsContext()
    test(ctx["incoming"] == "true")
    test(ctx["adapterName"] == "TestAdapter")
    test(ctx["remoteAddress"] == tcpinfo.localAddress)
    test(ctx["localAddress"] == tcpinfo.remoteAddress)
    test(ctx["remotePort"] == tcpinfo.localPort.to_s())
    test(ctx["localPort"] == tcpinfo.remotePort.to_s())

    if base.ice_getConnection().type() == "ws" || base.ice_getConnection().type() == "wss"
        test(info.is_a?(Ice::WSConnectionInfo))

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
