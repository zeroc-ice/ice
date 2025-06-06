// Copyright (c) ZeroC, Inc.

import IceImpl

final class EndpointInfoFactory: ICEEndpointInfoFactory {
    static func createTCPEndpointInfo(
        _ timeout: Int32,
        compress: Bool,
        host: String,
        port: Int32,
        sourceAddress: String,
        type: Int16,
        secure: Bool
    ) -> Any {
        TCPEndpointInfo(
            timeout: timeout,
            compress: compress,
            host: host,
            port: port,
            sourceAddress: sourceAddress,
            type: type,
            secure: secure)
    }

    static func createUDPEndpointInfo(
        _ compress: Bool,
        host: String,
        port: Int32,
        sourceAddress: String,
        mcastInterface: String,
        mcastTtl: Int32
    ) -> Any {
        UDPEndpointInfo(
            compress: compress,
            host: host,
            port: port,
            sourceAddress: sourceAddress,
            mcastInterface: mcastInterface,
            mcastTtl: mcastTtl)
    }

    static func createWSEndpointInfo(
        _ underlying: Any,
        resource: String
    ) -> Any {
        WSEndpointInfo(
            underlying: underlying as! EndpointInfo,
            resource: resource)
    }

    static func createSSLEndpointInfo(_ underlying: Any) -> Any {
        SSLEndpointInfo(underlying: underlying as! EndpointInfo)
    }

    static func createIAPEndpointInfo(
        _ timeout: Int32,
        compress: Bool,
        manufacturer: String,
        modelNumber: String,
        name: String,
        protocol: String,
        type: Int16,
        secure: Bool
    ) -> Any {
        IAPEndpointInfo(
            timeout: timeout,
            compress: compress,
            manufacturer: manufacturer,
            modelNumber: modelNumber,
            name: name,
            protocol: `protocol`,
            type: type,
            secure: secure)
    }

    static func createOpaqueEndpointInfo(
        _ type: Int16,
        encodingMajor: UInt8,
        encodingMinor: UInt8,
        rawBytes: Data
    ) -> Any {
        OpaqueEndpointInfo(
            type: type,
            rawEncoding: EncodingVersion(major: encodingMajor, minor: encodingMinor),
            rawBytes: rawBytes)
    }
}
