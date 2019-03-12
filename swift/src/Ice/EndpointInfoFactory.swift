// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class EndpointInfoFactory: ICEEndpointInfoFactory {
    static func createIPEndpointInfo(_ handle: ICEEndpointInfo,
                                     underlying: Any,
                                     timeout: Int32,
                                     compress: Bool,
                                     host: String,
                                     port: Int32,
                                     sourceAddress: String) -> Any {
        return IPEndpointInfoI(handle: handle,
                               underlying: getUnderlying(underlying),
                               timeout: timeout,
                               compress: compress,
                               host: host,
                               port: port,
                               sourceAddress: sourceAddress)
    }

    static func createTCPEndpointInfo(_ handle: ICEEndpointInfo,
                                      underlying: Any,
                                      timeout: Int32,
                                      compress: Bool,
                                      host: String,
                                      port: Int32,
                                      sourceAddress: String) -> Any {
        return TCPEndpointInfoI(handle: handle,
                                underlying: getUnderlying(underlying),
                                timeout: timeout,
                                compress: compress,
                                host: host,
                                port: port,
                                sourceAddress: sourceAddress)
    }

    static func createUDPEndpointInfo(_ handle: ICEEndpointInfo,
                                      underlying: Any,
                                      timeout: Int32,
                                      compress: Bool,
                                      host: String,
                                      port: Int32,
                                      sourceAddress: String,
                                      mcastInterface: String,
                                      mcastTtl: Int32) -> Any {
        return UDPEndpointInfoI(handle: handle,
                                underlying: getUnderlying(underlying),
                                timeout: timeout,
                                compress: compress,
                                host: host,
                                port: port,
                                sourceAddress: sourceAddress,
                                mcastInterface: mcastInterface,
                                mcastTtl: mcastTtl)
    }

    static func createWSEndpointInfo(_ handle: ICEEndpointInfo,
                                     underlying: Any,
                                     timeout: Int32,
                                     compress: Bool,
                                     resource: String) -> Any {
        return WSEndpointInfoI(handle: handle,
                               underlying: getUnderlying(underlying),
                               timeout: timeout,
                               compress: compress,
                               resource: resource)
    }

    static func createOpaqueEndpointInfo(_ handle: ICEEndpointInfo,
                                         underlying: Any,
                                         timeout: Int32,
                                         compress: Bool,
                                         encodingMajor: UInt8,
                                         encodingMinor _: UInt8,
                                         rawBytes: [NSNumber]) -> Any {
        return OpaqueEndpointInfoI(handle: handle,
                                   underlying: getUnderlying(underlying),
                                   timeout: timeout,
                                   compress: compress,
                                   rawEncoding: EncodingVersion(major: encodingMajor, minor: encodingMajor),
                                   rawBytes: rawBytes as! [UInt8])
    }

    static func createSSLEndpointInfo(_ handle: ICEEndpointInfo,
                                      underlying: Any,
                                      timeout: Int32,
                                      compress: Bool) -> Any {
        return SSLEndpointInfoI(handle: handle,
                                underlying: getUnderlying(underlying),
                                timeout: timeout,
                                compress: compress)
    }

    #if os(iOS) || os(watchOS) || os(tvOS)

    static func createIAPEndpointInfo(_ handle: ICEEndpointInfo,
                                      underlying: Any,
                                      timeout: Int32,
                                      compress: Bool,
                                      manufacturer: String,
                                      modelNumber: String,
                                      name: String,
                                      protocol: String) -> Any {
        return IAPEndpointInfoI(handle: handle,
                                underlying: getUnderlying(underlying),
                                timeout: timeout,
                                compress: compress,
                                manufacturer: manufacturer,
                                modelNumber: modelNumber,
                                name: name,
                                protocol: `protocol`)
    }

    #endif

    static func getUnderlying(_ info: Any) -> EndpointInfo? {
        return info as? EndpointInfo
    }
}
