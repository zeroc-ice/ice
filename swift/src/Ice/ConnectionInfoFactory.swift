// Copyright (c) ZeroC, Inc.

import Foundation
import IceImpl

final class ConnectionInfoFactory: ICEConnectionInfoFactory {
    static func createTCPConnectionInfo(
        _ incoming: Bool,
        adapterName: String,
        connectionId: String,
        localAddress: String,
        localPort: Int32,
        remoteAddress: String,
        remotePort: Int32,
        rcvSize: Int32,
        sndSize: Int32
    ) -> Any {
        TCPConnectionInfo(
            incoming: incoming,
            adapterName: adapterName,
            connectionId: connectionId,
            localAddress: localAddress,
            localPort: localPort,
            remoteAddress: remoteAddress,
            remotePort: remotePort,
            rcvSize: rcvSize,
            sndSize: sndSize)
    }

    static func createUDPConnectionInfo(
        _ incoming: Bool,
        adapterName: String,
        connectionId: String,
        localAddress: String,
        localPort: Int32,
        remoteAddress: String,
        remotePort: Int32,
        mcastAddress: String,
        mcastPort: Int32,
        rcvSize: Int32,
        sndSize: Int32
    ) -> Any {
        UDPConnectionInfo(
            incoming: incoming,
            adapterName: adapterName,
            connectionId: connectionId,
            localAddress: localAddress,
            localPort: localPort,
            remoteAddress: remoteAddress,
            remotePort: remotePort,
            mcastAddress: mcastAddress,
            mcastPort: mcastPort,
            rcvSize: rcvSize,
            sndSize: sndSize)
    }

    static func createWSConnectionInfo(_ underlying: Any, headers: [String: String]) -> Any {
        WSConnectionInfo(underlying: underlying as! ConnectionInfo, headers: headers)
    }

    static func createSSLConnectionInfo(_ underlying: Any, peerCertificate: SecCertificate) -> Any {
        return SSLConnectionInfo(
            underlying: underlying as! ConnectionInfo, peerCertificate: peerCertificate)
    }

    static func createIAPConnectionInfo(
        _ incoming: Bool,
        adapterName: String,
        connectionId: String,
        name: String,
        manufacturer: String,
        modelNumber: String,
        firmwareRevision: String,
        hardwareRevision: String,
        protocol: String
    ) -> Any {
        IAPConnectionInfo(
            incoming: incoming,
            adapterName: adapterName,
            connectionId: connectionId,
            name: name,
            manufacturer: manufacturer,
            modelNumber: modelNumber,
            firmwareRevision: firmwareRevision,
            hardwareRevision: hardwareRevision,
            protocol: `protocol`)
    }
}
