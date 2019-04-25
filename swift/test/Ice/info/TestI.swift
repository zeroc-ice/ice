//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice

func getIPEndpointInfo(_ info: Ice.EndpointInfo) -> Ice.IPEndpointInfo? {
    var curr: Ice.EndpointInfo? = info
    while curr != nil {
        if curr is Ice.IPEndpointInfo {
            return curr as? Ice.IPEndpointInfo
        }
        curr = curr?.underlying
    }
    return nil
}

func getIPConnectionInfo(_ info: Ice.ConnectionInfo) -> Ice.IPConnectionInfo? {
    var curr: Ice.ConnectionInfo? = info
    while curr != nil {
        if info is Ice.IPConnectionInfo {
            return info as? Ice.IPConnectionInfo
        }
        curr = curr?.underlying
    }
    return nil
}

class TestI: TestIntf {
    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }

    func getEndpointInfoAsContext(current: Ice.Current) throws -> Ice.Context {
        var ctx = Ice.Context()
        let info = current.con!.getEndpoint().getInfo()!
        ctx["timeout"] = "\(info.timeout)"
        ctx["compress"] = info.compress ? "true" : "false"
        ctx["datagram"] = info.datagram() ? "true" : "false"
        ctx["secure"] = info.datagram() ? "true" : "false"
        ctx["type"] = "\(info.type())"

        let ipinfo = getIPEndpointInfo(info)!
        ctx["host"] = ipinfo.host
        ctx["port"] = "\(ipinfo.port)"

        if let udpinfo = ipinfo as? Ice.UDPEndpointInfo {
            ctx["mcastInterface"] = udpinfo.mcastInterface
            ctx["mcastTtl"] = "\(udpinfo.mcastTtl)"
        }
        return ctx
    }

    func getConnectionInfoAsContext(current: Ice.Current) throws -> Ice.Context {
        var ctx = Ice.Context()
        let info = try current.con!.getInfo()
        ctx["adapterName"] = info.adapterName
        ctx["incoming"] = info.incoming ? "true" : "false"

        let ipinfo = getIPConnectionInfo(info)!
        ctx["localAddress"] = ipinfo.localAddress
        ctx["localPort"] = "\(ipinfo.localPort)"
        ctx["remoteAddress"] = ipinfo.remoteAddress
        ctx["remotePort"] = "\(ipinfo.remotePort)"

        if let wsinfo = info as? WSConnectionInfo {
            for (key, value) in wsinfo.headers {
                ctx["ws.\(key)"] = value
            }
        }
        return ctx
    }
}
