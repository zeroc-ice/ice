//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceObjc

class EndpointI: LocalObject<ICEEndpoint>, Endpoint {
    func toString() -> String {
        return handle.toString()
    }

    func getInfo() -> EndpointInfo? {
        return handle.getInfo() as? EndpointInfo
    }

    // CustomStringConvertible implementation
    var description: String {
        return toString()
    }
}

public func != (lhs: Endpoint?, rhs: Endpoint?) -> Bool {
    return !(lhs == rhs)
}

public func == (lhs: Endpoint?, rhs: Endpoint?) -> Bool {
    if lhs === rhs {
        return true
    } else if lhs === nil && rhs === nil {
        return true
    } else if lhs === nil || rhs === nil {
        return false
    } else {
        let lhsI = lhs as! EndpointI
        let rhsI = rhs as! EndpointI
        return lhsI.handle.isEqual(rhsI.handle)
    }
}

class EndpointInfoI: LocalObject<ICEEndpointInfo>, EndpointInfo {
    var underlying: EndpointInfo?
    var timeout: Int32
    var compress: Bool

    init(handle: ICEEndpointInfo, underlying: EndpointInfo?, timeout: Int32, compress: Bool) {
        self.underlying = underlying
        self.timeout = timeout
        self.compress = compress
        super.init(handle: handle)
    }

    func type() -> Int16 {
        return underlying?.type() ?? handle.getType()
    }

    func datagram() -> Bool {
        return underlying?.datagram() ?? handle.getDatagram()
    }

    func secure() -> Bool {
        return underlying?.secure() ?? handle.getSecure()
    }
}

class IPEndpointInfoI: EndpointInfoI, IPEndpointInfo {
    var host: String
    var port: Int32
    var sourceAddress: String

    init(handle: ICEEndpointInfo,
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
    var mcastInterface: String
    var mcastTtl: Int32

    init(handle: ICEEndpointInfo,
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
    var resource: String

    init(handle: ICEEndpointInfo, underlying: EndpointInfo?, timeout: Int32, compress: Bool, resource: String) {
        self.resource = resource
        super.init(handle: handle, underlying: underlying, timeout: timeout, compress: compress)
    }
}

class OpaqueEndpointInfoI: EndpointInfoI, OpaqueEndpointInfo {
    var rawEncoding: EncodingVersion
    var rawBytes: ByteSeq

    init(handle: ICEEndpointInfo,
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

#if os(iOS) || os(watchOS) || os(tvOS)

    // IceIAP (iOS only)
    class IAPEndpointInfoI: EndpointInfoI, IAPEndpointInfo {
        var manufacturer: String
        var modelNumber: String
        var name: String
        var `protocol`: String

        init(handle: ICEEndpointInfo, underlying: EndpointInfo?, timeout: Int32, compress: Bool,
             manufacturer: String, modelNumber: String, name: String, protocol: String) {
            self.manufacturer = manufacturer
            self.modelNumber = modelNumber
            self.name = name
            self.protocol = `protocol`
            super.init(handle: handle, underlying: underlying, timeout: timeout, compress: compress)
        }
    }

#endif

//
// Internal helpers to convert from ObjC to Swift objects
//
extension Array where Element == ICEEndpoint {
    func fromObjc() -> EndpointSeq {
        return map { objcEndpt in
            objcEndpt.getSwiftObject(EndpointI.self) {
                EndpointI(handle: objcEndpt)
            }
        }
    }
}

extension Array where Element == Endpoint {
    func toObjc() -> [ICEEndpoint] {
        return map { endpt in
            (endpt as! EndpointI).handle
        }
    }
}
