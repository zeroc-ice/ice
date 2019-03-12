// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class EndpointI: LocalObject<ICEEndpoint>, Endpoint, CustomStringConvertible {
    func toString() -> String {
        return _handle.toString()
    }

    func getInfo() -> EndpointInfo? {
        return _handle.getInfo() as? EndpointInfo
    }
}

extension Endpoint where Self: CustomStringConvertible {
    var description: String {
        return toString()
    }
}

class EndpointInfoI: LocalObject<ICEEndpointInfo>, EndpointInfo {
    public var underlying: EndpointInfo?
    public var timeout: Int32
    public var compress: Bool

    init(handle: ICEEndpointInfo, underlying: EndpointInfo?, timeout: Int32, compress: Bool) {
        self.underlying = underlying
        self.timeout = timeout
        self.compress = compress
        super.init(handle: handle)
    }

    func type() -> Int16 {
        return underlying?.type() ?? _handle.getType()
    }

    func datagram() -> Bool {
        return underlying?.datagram() ?? _handle.getDatagram()
    }

    func secure() -> Bool {
        return underlying?.secure() ?? _handle.getSecure()
    }
}

class IPEndpointInfoI: EndpointInfoI, IPEndpointInfo {
    public var host: String
    public var port: Int32
    public var sourceAddress: String

    public init(handle: ICEEndpointInfo,
                underlying: EndpointInfo?,
                timeout: Int32,
                compress: Bool,
                host: String,
                port: Int32,
                sourceAddress: String) {
        self.host = host
        self.port = port
        self.sourceAddress = sourceAddress
        super.init(handle: handle, underlying: underlying, timeout: timeout, compress: compress)
    }
}

class TCPEndpointInfoI: IPEndpointInfoI, TCPEndpointInfo {}

class UDPEndpointInfoI: IPEndpointInfoI, UDPEndpointInfo {
    public var mcastInterface: String
    public var mcastTtl: Int32

    public init(handle: ICEEndpointInfo,
                underlying: EndpointInfo?,
                timeout: Int32,
                compress: Bool,
                host: String,
                port: Int32,
                sourceAddress: String,
                mcastInterface: String,
                mcastTtl: Int32) {
        self.mcastInterface = mcastInterface
        self.mcastTtl = mcastTtl
        super.init(handle: handle,
                   underlying: underlying,
                   timeout: timeout,
                   compress: compress,
                   host: host,
                   port: port,
                   sourceAddress: sourceAddress)
    }
}

class WSEndpointInfoI: EndpointInfoI, WSEndpointInfo {
    public var resource: String

    public init(handle: ICEEndpointInfo, underlying: EndpointInfo?, timeout: Int32, compress: Bool, resource: String) {
        self.resource = resource
        super.init(handle: handle, underlying: underlying, timeout: timeout, compress: compress)
    }
}

class OpaqueEndpointInfoI: EndpointInfoI, OpaqueEndpointInfo {
    public var rawEncoding: EncodingVersion
    public var rawBytes: ByteSeq

    public init(handle: ICEEndpointInfo,
                underlying: EndpointInfo?,
                timeout: Int32,
                compress: Bool,
                rawEncoding: EncodingVersion,
                rawBytes: ByteSeq) {
        self.rawEncoding = rawEncoding
        self.rawBytes = rawBytes
        super.init(handle: handle, underlying: underlying, timeout: timeout, compress: compress)
    }
}

//
// IceSSL
//
class SSLEndpointInfoI: EndpointInfoI, SSLEndpointInfo {}

#if os(iOS)

//
// IceIAP (iOS only)
//
class IAPEndpointInfoI: EndpointInfoI, IAPEndpointInfo {
    var manufacturer: String
    var modelNumber: String
    var name: String
    var _protocol: String

    public init(handle: ICEEndpointInfo, underlying: EndpointInfo?, timeout: Int32, compress: Bool, manufacturer: String, modelNumber: String, name: String, protocol: String) {
        self.manufacturer = manufacturer
        self.modelNumber = modelNumber
        self.name = name
        self._protocol = `protocol`
        super.init(handle: handle, underlying: underlying, timeout: timeout, compress: compress)
    }
}

#endif
