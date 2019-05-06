//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

func getTCPEndpointInfo(_ info: Ice.EndpointInfo) -> Ice.TCPEndpointInfo? {
    var curr: Ice.EndpointInfo? = info
    while curr != nil {
        if curr is Ice.TCPEndpointInfo {
            return curr as? Ice.TCPEndpointInfo
        }
        curr = curr?.underlying
    }
    return nil
}

func getTCPConnectionInfo(_ info: Ice.ConnectionInfo) -> Ice.TCPConnectionInfo? {
    var curr: Ice.ConnectionInfo? = info
    while curr != nil {
        if curr is Ice.TCPConnectionInfo {
            return curr as? Ice.TCPConnectionInfo
        }
        curr = curr?.underlying
    }
    return nil
}

func allTests(_ helper: TestHelper) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()

    let communicator = helper.communicator()

    output.write("testing proxy endpoint information... ")
    do {
        let p1 = try communicator.stringToProxy(
            "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" +
            "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" +
            "opaque -e 1.8 -t 100 -v ABCD")!

        var endps = p1.ice_getEndpoints()

        let info = endps[0].getInfo()!
        let tcpEndpoint = getTCPEndpointInfo(info)!
        try test(tcpEndpoint.host == "tcphost")
        try test(tcpEndpoint.port == 10000)
        try test(tcpEndpoint.sourceAddress == "10.10.10.10")
        try test(tcpEndpoint.timeout == 1200)
        try test(tcpEndpoint.compress)
        try test(!tcpEndpoint.datagram())

        try test(tcpEndpoint.type() == Ice.TCPEndpointType && !tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.SSLEndpointType && tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.WSEndpointType && !tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.WSSEndpointType && tcpEndpoint.secure())

        try test(tcpEndpoint.type() == Ice.TCPEndpointType && info is Ice.TCPEndpointInfo ||
                 tcpEndpoint.type() == Ice.SSLEndpointType && info is Ice.SSLEndpointInfo ||
                 tcpEndpoint.type() == Ice.WSEndpointType && info is Ice.WSEndpointInfo ||
                 tcpEndpoint.type() == Ice.WSSEndpointType && info is Ice.WSEndpointInfo)

        let udpEndpoint = (endps[1].getInfo() as? Ice.UDPEndpointInfo)!
        try test(udpEndpoint.host == "udphost")
        try test(udpEndpoint.port == 10001)
        try test(udpEndpoint.mcastInterface == "eth0")
        try test(udpEndpoint.mcastTtl == 5)
        try test(udpEndpoint.sourceAddress == "10.10.10.10")
        try test(udpEndpoint.timeout == -1)
        try test(!udpEndpoint.compress)
        try test(!udpEndpoint.secure())
        try test(udpEndpoint.datagram())
        try test(udpEndpoint.type() == 3)

        let opaqueEndpoint = (endps[2].getInfo() as? Ice.OpaqueEndpointInfo)!
        try test(opaqueEndpoint.rawBytes.count > 0)
        try test(opaqueEndpoint.rawEncoding == Ice.EncodingVersion(major: 1, minor: 8))
    }
    output.writeLine("ok")

    output.write("test object adapter endpoint information... ")
    do {
        let host = communicator.getProperties().getPropertyAsInt("Ice.IPv6") != 0 ? "::1" : "127.0.0.1"
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints",
                                                 value: "tcp -h \"\(host)\" -t 15000:udp -h \"\(host)\"")
        var adapter = try communicator.createObjectAdapter("TestAdapter")

        var endpoints = adapter.getEndpoints()
        try test(endpoints.count == 2)
        var publishedEndpoints = adapter.getPublishedEndpoints()

        try test(endpoints.elementsEqual(publishedEndpoints) { $0 == $1})

        var tcpEndpoint = getTCPEndpointInfo(endpoints[0].getInfo()!)!
        try test(tcpEndpoint.type() == Ice.TCPEndpointType ||
                 tcpEndpoint.type() == Ice.SSLEndpointType ||
                 tcpEndpoint.type() == Ice.WSEndpointType ||
                 tcpEndpoint.type() == Ice.WSSEndpointType)

        try test(tcpEndpoint.host == host)
        try test(tcpEndpoint.port > 0)
        try test(tcpEndpoint.timeout == 15000)

        let udpEndpoint = (endpoints[1].getInfo() as? Ice.UDPEndpointInfo)!
        try test(udpEndpoint.host == host)
        try test(udpEndpoint.datagram())
        try test(udpEndpoint.port > 0)

        endpoints = [endpoints[0]]
        try adapter.setPublishedEndpoints(endpoints)
        publishedEndpoints = adapter.getPublishedEndpoints()
        try test(endpoints.elementsEqual(publishedEndpoints) { $0 == $1})

        adapter.destroy()

        let port = helper.getTestPort(num: 1)
        communicator.getProperties().setProperty(key: "TestAdapter.Endpoints", value: "default -h * -p \(port)")
        communicator.getProperties().setProperty(key: "TestAdapter.PublishedEndpoints",
                                                 value: helper.getTestEndpoint(num: 1))
        adapter = try communicator.createObjectAdapter("TestAdapter")

        endpoints = adapter.getEndpoints()
        try test(endpoints.count >= 1)
        publishedEndpoints = adapter.getPublishedEndpoints()
        try test(publishedEndpoints.count == 1)

        for endpoint in endpoints {
            tcpEndpoint = getTCPEndpointInfo(endpoint.getInfo()!)!
            try test(tcpEndpoint.port == port)
        }

        tcpEndpoint = getTCPEndpointInfo(publishedEndpoints[0].getInfo()!)!
        try test(tcpEndpoint.host == helper.getTestHost())
        try test(tcpEndpoint.port == port)

        adapter.destroy()
    }
    output.writeLine("ok")

    let endpointPort = helper.getTestPort(num: 0)

    let base = try communicator.stringToProxy("test:" +
        helper.getTestEndpoint(num: 0) + ":" +
        helper.getTestEndpoint(num: 0, prot: "udp"))!

    let testIntf = try checkedCast(prx: base, type: TestIntfPrx.self)!

    let defaultHost = communicator.getProperties().getProperty("Ice.Default.Host")

    output.write("test connection endpoint information... ")
    do {
        var info = try base.ice_getConnection()!.getEndpoint().getInfo()!
        let tcpinfo = getTCPEndpointInfo(info)!
        try test(tcpinfo.port == endpointPort)
        try test(!tcpinfo.compress)
        try test(tcpinfo.host == defaultHost)

        let ctx = try testIntf.getEndpointInfoAsContext()
        try test(ctx["host"] == tcpinfo.host)
        try test(ctx["compress"] == "false")
        let port = Int(ctx["port"]!)!
        try test(port > 0)

        info = try base.ice_datagram().ice_getConnection()!.getEndpoint().getInfo()!
        let udp = (info as? Ice.UDPEndpointInfo)!
        try test(udp.port == endpointPort)
        try test(udp.host == defaultHost)
    }
    output.writeLine("ok")

    output.write("testing connection information... ")
    do {
        var connection = try base.ice_getConnection()!
        try connection.setBufferSize(rcvSize: 1024, sndSize: 2048)

        let info = try connection.getInfo()
        let ipInfo = getTCPConnectionInfo(info)!
        try test(!info.incoming)
        try test(info.adapterName == "")
        try test(ipInfo.remotePort == endpointPort)
        try test(ipInfo.localPort > 0)

        if defaultHost == "127.0.0.1" {
            try test(ipInfo.localAddress == defaultHost)
            try test(ipInfo.remoteAddress == defaultHost)
        }
        try test(ipInfo.rcvSize >= 1024)
        try test(ipInfo.sndSize >= 2048)

        let ctx = try testIntf.getConnectionInfoAsContext()
        try test(ctx["incoming"] == "true")
        try test(ctx["adapterName"] == "TestAdapter")
        try test(ctx["remoteAddress"] == ipInfo.localAddress)
        try test(ctx["localAddress"] == ipInfo.remoteAddress)
        try test(ctx["remotePort"] == "\(ipInfo.localPort)")
        try test(ctx["localPort"] == "\(ipInfo.remotePort)")

        if try base.ice_getConnection()!.type() == "ws" ||
               base.ice_getConnection()!.type() == "wss" {

            let headers = (info as? Ice.WSConnectionInfo)!.headers

            try test(headers["Upgrade"] == "websocket")
            try test(headers["Connection"] == "Upgrade")
            try test(headers["Sec-WebSocket-Protocol"] == "ice.zeroc.com")
            try test(headers["Sec-WebSocket-Accept"] != nil)

            try test(ctx["ws.Upgrade"] == "websocket")
            try test(ctx["ws.Connection"] == "Upgrade")
            try test(ctx["ws.Sec-WebSocket-Protocol"] == "ice.zeroc.com")
            try test(ctx["ws.Sec-WebSocket-Version"] == "13")
            try test(ctx["ws.Sec-WebSocket-Key"] != nil)
        }

        connection = try base.ice_datagram().ice_getConnection()!
        try connection.setBufferSize(rcvSize: 2048, sndSize: 1024)

        let udpInfo = (try connection.getInfo() as? Ice.UDPConnectionInfo)!
        try test(!udpInfo.incoming)
        try test(udpInfo.adapterName == "")
        try test(udpInfo.localPort > 0)
        try test(udpInfo.remotePort == endpointPort)

        if defaultHost == "127.0.0.1" {
            try test(udpInfo.remoteAddress == defaultHost)
            try test(udpInfo.localAddress == defaultHost)
        }
        try test(udpInfo.rcvSize >= 2048)
        try test(udpInfo.sndSize >= 1024)
    }
    output.writeLine("ok")

    try testIntf.shutdown()
}
