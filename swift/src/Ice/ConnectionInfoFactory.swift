//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc
import Foundation

private class ConnectionInfoI: ConnectionInfo {
    var underlying: ConnectionInfo?
    var incoming: Bool
    var adapterName: String
    var connectionId: String

    init(underlying: ConnectionInfo?, incoming: Bool, adapterName: String, connectionId: String) {
        self.underlying = underlying
        self.incoming = incoming
        self.adapterName = adapterName
        self.connectionId = connectionId
    }
}

private class IPConnectionInfoI: ConnectionInfoI, IPConnectionInfo {
    var localAddress: String
    var localPort: Int32
    var remoteAddress: String
    var remotePort: Int32

    init(underlying: ConnectionInfo?, incoming: Bool, adapterName: String, connectionId: String,
         localAddress: String, localPort: Int32, remoteAddress: String, remotePort: Int32) {
        self.localAddress = localAddress
        self.localPort = localPort
        self.remoteAddress = remoteAddress
        self.remotePort = remotePort
        super.init(underlying: underlying, incoming: incoming, adapterName: adapterName, connectionId: connectionId)
    }
}

private class TCPConnectionInfoI: IPConnectionInfoI, TCPConnectionInfo {
    var rcvSize: Int32
    var sndSize: Int32

    init(underlying: ConnectionInfo?, incoming: Bool, adapterName: String, connectionId: String,
         localAddress: String, localPort: Int32, remoteAddress: String, remotePort: Int32,
         rcvSize: Int32, sndSize: Int32) {
        self.rcvSize = rcvSize
        self.sndSize = sndSize
        super.init(underlying: underlying, incoming: incoming, adapterName: adapterName, connectionId: connectionId,
                   localAddress: localAddress, localPort: localPort,
                   remoteAddress: remoteAddress, remotePort: remotePort)
    }
}

private class UDPConnectionInfoI: IPConnectionInfoI, UDPConnectionInfo {
    var mcastAddress: String
    var mcastPort: Int32
    var rcvSize: Int32
    var sndSize: Int32

    init(underlying: ConnectionInfo?, incoming: Bool, adapterName: String, connectionId: String,
         localAddress: String, localPort: Int32, remoteAddress: String, remotePort: Int32,
         mcastAddress: String, mcastPort: Int32, rcvSize: Int32, sndSize: Int32) {
        self.mcastAddress = mcastAddress
        self.mcastPort = mcastPort
        self.rcvSize = rcvSize
        self.sndSize = sndSize
        super.init(underlying: underlying, incoming: incoming, adapterName: adapterName, connectionId: connectionId,
                   localAddress: localAddress, localPort: localPort,
                   remoteAddress: remoteAddress, remotePort: remotePort)
    }
}

private class WSConnectionInfoI: ConnectionInfoI, WSConnectionInfo {
    var headers: HeaderDict

    init(underlying: ConnectionInfo?, incoming: Bool, adapterName: String, connectionId: String,
         headers: HeaderDict) {
        self.headers = headers
        super.init(underlying: underlying, incoming: incoming, adapterName: adapterName, connectionId: connectionId)
    }
}

private class SSLConnectionInfoI: ConnectionInfoI, SSLConnectionInfo {
    var cipher: String
    var certs: [SecCertificate]
    var verified: Bool

    init(underlying: ConnectionInfo?, incoming: Bool, adapterName: String, connectionId: String,
         cipher: String, certs: StringSeq, verified: Bool) {
        self.cipher = cipher
        self.certs = []
        let beginPrefix = "-----BEGIN CERTIFICATE-----\n"
        let endPrefix = "\n-----END CERTIFICATE-----\n"

        for cert in certs {
            var raw = cert
            if raw.hasPrefix(beginPrefix) {
                raw = String(raw.dropFirst(beginPrefix.count))
                raw = String(raw.dropLast(endPrefix.count))
            }

            if let data = NSData(base64Encoded: raw, options: .ignoreUnknownCharacters) {
                if let c = SecCertificateCreateWithData(kCFAllocatorDefault, data) {
                    self.certs.append(c)
                }
            }
        }
        self.verified = verified
        super.init(underlying: underlying, incoming: incoming, adapterName: adapterName, connectionId: connectionId)
    }
}

#if os(iOS) || os(watchOS) || os(tvOS)
    private class IAPConnectionInfoI: ConnectionInfoI, IAPConnectionInfo {
        var name: String
        var manufacturer: String
        var modelNumber: String
        var firmwareRevision: String
        var hardwareRevision: String
        var `protocol`: String

        init(underlying: ConnectionInfo?, incoming: Bool, adapterName: String, connectionId: String,
             name: String, manufacturer: String, modelNumber: String, firmwareRevision: String,
             hardwareRevision: String,
             protocol: String) {
            self.name = name
            self.manufacturer = manufacturer
            self.modelNumber = modelNumber
            self.firmwareRevision = firmwareRevision
            self.hardwareRevision = hardwareRevision
            self.protocol = `protocol`
            super.init(underlying: underlying, incoming: incoming, adapterName: adapterName, connectionId: connectionId)
        }
    }
#endif

class ConnectionInfoFactory: ICEConnectionInfoFactory {
    static func createIPConnectionInfo(_ underlying: Any,
                                       incoming: Bool,
                                       adapterName: String,
                                       connectionId: String,
                                       localAddress: String,
                                       localPort: Int32,
                                       remoteAddress: String,
                                       remotePort: Int32) -> Any {
        return IPConnectionInfoI(underlying: getUnderlying(underlying),
                                 incoming: incoming,
                                 adapterName: adapterName,
                                 connectionId: connectionId,
                                 localAddress: localAddress,
                                 localPort: localPort,
                                 remoteAddress: remoteAddress,
                                 remotePort: remotePort)
    }

    static func createTCPConnectionInfo(_ underlying: Any,
                                        incoming: Bool,
                                        adapterName: String,
                                        connectionId: String,
                                        localAddress: String,
                                        localPort: Int32,
                                        remoteAddress: String,
                                        remotePort: Int32,
                                        rcvSize: Int32,
                                        sndSize: Int32) -> Any {
        return TCPConnectionInfoI(underlying: getUnderlying(underlying), incoming: incoming,
                                  adapterName: adapterName,
                                  connectionId: connectionId,
                                  localAddress: localAddress,
                                  localPort: localPort,
                                  remoteAddress: remoteAddress,
                                  remotePort: remotePort,
                                  rcvSize: rcvSize,
                                  sndSize: sndSize)
    }

    static func createUDPConnectionInfo(_ underlying: Any,
                                        incoming: Bool,
                                        adapterName: String,
                                        connectionId: String,
                                        localAddress: String,
                                        localPort: Int32,
                                        remoteAddress: String,
                                        remotePort: Int32, mcastAddress: String, mcastPort: Int32,
                                        rcvSize: Int32,
                                        sndSize: Int32) -> Any {
        return UDPConnectionInfoI(underlying: getUnderlying(underlying), incoming: incoming,
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

    static func createWSConnectionInfo(_ underlying: Any,
                                       incoming: Bool,
                                       adapterName: String,
                                       connectionId: String,
                                       headers: [String: String]) -> Any {
        return WSConnectionInfoI(underlying: getUnderlying(underlying),
                                 incoming: incoming,
                                 adapterName: adapterName,
                                 connectionId: connectionId,
                                 headers: headers)
    }

    static func createSSLConnectionInfo(_ underlying: Any,
                                        incoming: Bool,
                                        adapterName: String,
                                        connectionId: String,
                                        cipher: String,
                                        certs: [String], verified: Bool) -> Any {
        return SSLConnectionInfoI(underlying: getUnderlying(underlying),
                                  incoming: incoming,
                                  adapterName: adapterName,
                                  connectionId: connectionId,
                                  cipher: cipher,
                                  certs: certs,
                                  verified: verified)
    }

    #if os(iOS) || os(watchOS) || os(tvOS)

        static func createIAPConnectionInfo(_ underlying: Any,
                                            incoming: Bool,
                                            adapterName: String,
                                            connectionId: String,
                                            name: String,
                                            manufacturer: String,
                                            modelNumber: String,
                                            firmwareRevision: String,
                                            hardwareRevision: String,
                                            protocol: String) -> Any {
            return IAPConnectionInfoI(underlying: getUnderlying(underlying),
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

    #endif

    static func getUnderlying(_ info: Any) -> ConnectionInfo? {
        return info as? ConnectionInfo
    }
}
