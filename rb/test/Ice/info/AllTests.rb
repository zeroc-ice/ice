# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def allTests(communicator)
    print "testing proxy endpoint information..."
    STDOUT.flush

    p1 = communicator.stringToProxy("test -t:default -h tcphost -p 10000 -t 1200 -z:" + \
                                    "udp -h udphost -p 10001 --interface eth0 --ttl 5:" + \
                                    "opaque -e 1.8 -t 100 -v ABCD")

    endps = p1.ice_getEndpoints()

    ipEndpoint = endps[0].getInfo()
    test(ipEndpoint.is_a?(Ice::IPEndpointInfo));
    test(ipEndpoint.host == "tcphost")
    test(ipEndpoint.port == 10000)
    test(ipEndpoint.timeout == 1200)
    test(ipEndpoint.compress)
    test(!ipEndpoint.datagram())
    test((ipEndpoint.type() == Ice::TCPEndpointType && !ipEndpoint.secure()) ||
         (ipEndpoint.type() == 2 && ipEndpoint.secure()))
    test((ipEndpoint.type() == Ice::TCPEndpointType && ipEndpoint.is_a?(Ice::TCPEndpointInfo)) ||
         (ipEndpoint.type() == 2))

    udpEndpoint = endps[1].getInfo()
    test(udpEndpoint.is_a?(Ice::UDPEndpointInfo));
    test(udpEndpoint.host == "udphost")
    test(udpEndpoint.port == 10001)
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

    info = base.ice_getConnection().getInfo()
    test(!info.incoming)
    test(info.adapterName.length == 0)
    test(info.remotePort == 12010)
    test(info.remoteAddress == defaultHost)
    test(info.localAddress == defaultHost)

    ctx = testIntf.getConnectionInfoAsContext()
    test(ctx["incoming"] == "true")
    test(ctx["adapterName"] == "TestAdapter")
    test(ctx["remoteAddress"] == info.localAddress)
    test(ctx["localAddress"] == info.remoteAddress)
    test(ctx["remotePort"] == info.localPort.to_s())
    test(ctx["localPort"] == info.remotePort.to_s())

    puts "ok"

    testIntf.shutdown()

    communicator.shutdown()
end
