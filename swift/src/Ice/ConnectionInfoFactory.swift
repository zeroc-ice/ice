// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class ConnectionInfoFactory: ICEConnectionInfoFactory {

    static func createIPConnectionInfo(_ underlying: Any, incoming: Bool,adapterName: String, connectionId: String, localAddress: String, localPort: Int32, remoteAddress: String, remotePort: Int32) -> Any {
        return IPConnectionInfo(underlying: getUnderlying(underlying), incoming: incoming, adapterName: adapterName, connectionId: connectionId, localAddress: localAddress, localPort: localPort, remoteAddress: remoteAddress, remotePort: remotePort)
    }

    static func createTCPConnectionInfo(_ underlying: Any, incoming: Bool,adapterName: String, connectionId: String, localAddress: String, localPort: Int32, remoteAddress: String, remotePort: Int32, rcvSize: Int32, sndSize: Int32) -> Any {
        return TCPConnectionInfo(underlying: getUnderlying(underlying), incoming: incoming, adapterName: adapterName, connectionId: connectionId, localAddress: localAddress, localPort: localPort, remoteAddress: remoteAddress, remotePort: remotePort, rcvSize: rcvSize, sndSize: sndSize)
    }

    static func createUDPConnectionInfo(_ underlying: Any, incoming: Bool,adapterName: String, connectionId: String, localAddress: String, localPort: Int32, remoteAddress: String, remotePort: Int32, mcastAddress: String, mcastPort: Int32, rcvSize: Int32, sndSize: Int32) -> Any {
        return UDPConnectionInfo(underlying: getUnderlying(underlying), incoming: incoming, adapterName: adapterName, connectionId: connectionId, localAddress: localAddress, localPort: localPort, remoteAddress: remoteAddress, remotePort: remotePort, mcastAddress: mcastAddress, mcastPort: mcastPort, rcvSize: rcvSize, sndSize: sndSize)
    }

    static func createWSConnectionInfo(_ underlying: Any, incoming: Bool,adapterName: String, connectionId: String, headers: [String : String]) -> Any {
        return WSConnectionInfo(underlying: getUnderlying(underlying), incoming: incoming, adapterName: adapterName, connectionId: connectionId, headers: headers)
    }

    static func createSSLConnectionInfo(_ underlying: Any, incoming: Bool,adapterName: String, connectionId: String, cipher: String, certs: [String], verified: Bool) -> Any {
        return SSLConnectionInfo(underlying: getUnderlying(underlying), incoming: incoming, adapterName: adapterName, connectionId: connectionId, cipher: cipher, certs: certs, verified: verified)
    }

    static func getUnderlying(_ info: Any) -> ConnectionInfo? {
        return info as? ConnectionInfo
    }

}

// TODO add iAP
