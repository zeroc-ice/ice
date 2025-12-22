// Copyright (c) ZeroC, Inc.

import Foundation

/// Represents batch compression options when flushing queued batch requests.
public enum CompressBatch: UInt8 {
    /// Compress the batch requests.
    case Yes = 0

    /// Don't compress the batch requests.
    case No = 1

    /// Compress the batch requests if at least one request was made on a compressed proxy.
    case BasedOnProxy = 2

    public init() {
        self = .Yes
    }
}

/// An `Ice.InputStream` extension to read `CompressBatch` enumerated values from the stream.
extension InputStream {
    /// Read an enumerated value.
    ///
    /// - Returns: The enumerated value.
    public func read() throws -> CompressBatch {
        let rawValue: UInt8 = try read(enumMaxValue: 2)
        guard let val = CompressBatch(rawValue: rawValue) else {
            throw MarshalException("invalid enum value")
        }
        return val
    }

    /// Read an optional enumerated value from the stream.
    ///
    /// - Parameter tag: The numeric tag associated with the value.
    /// - Returns: The enumerated value.
    public func read(tag: Int32) throws -> CompressBatch? {
        guard try readOptional(tag: tag, expectedFormat: .Size) else {
            return nil
        }
        return try read() as CompressBatch
    }
}

/// An `Ice.OutputStream` extension to write `CompressBatch` enumerated values to the stream.
extension OutputStream {
    /// Writes an enumerated value to the stream.
    ///
    /// - Parameter v: The enumerator to write.
    public func write(_ v: CompressBatch) {
        write(enum: v.rawValue, maxValue: 2)
    }

    /// Writes an optional enumerated value to the stream.
    ///
    /// - Parameters:
    ///   - tag: The numeric tag associated with the value.
    ///   - value: The enumerator to write.
    public func write(tag: Int32, value: CompressBatch?) {
        guard let v = value else {
            return
        }
        write(tag: tag, val: v.rawValue, maxValue: 2)
    }
}

/// Represents a collection of HTTP headers.
public typealias HeaderDict = [String: String]

/// The callback function given to ``Connection/setCloseCallback(_:)``.
/// This callback is called by the connection when the connection is closed.
///
/// - Parameter _: The connection that was closed. It's never `nil`.
public typealias CloseCallback = (Connection?) -> Void

/// Represents a connection that uses the Ice protocol.
public protocol Connection: AnyObject, CustomStringConvertible, Sendable {
    /// Aborts this connection.
    func abort()

    /// Closes this connection gracefully once all outstanding invocations have completed. If closing the connection
    /// takes longer than the configured close timeout, the connection is aborted with a ``CloseTimeoutException``.
    func close() async throws

    /// Creates a special proxy (a "fixed proxy") that always uses this connection.
    ///
    /// - Parameter id: The identity of the target object.
    /// - Returns: A fixed proxy with the provided identity.
    func createProxy(_ id: Identity) throws -> ObjectPrx

    /// Associates an object adapter with this connection. When a connection receives a request, it dispatches this
    /// request using its associated object adapter. If the associated object adapter is `nil`, the connection
    /// rejects any incoming request with an ``ObjectNotExistException``.
    /// The default object adapter of an incoming connection is the object adapter that created this connection;
    /// the default object adapter of an outgoing connection is the communicator's default object adapter.
    ///
    /// - Parameter adapter: The object adapter to associate with this connection.
    func setAdapter(_ adapter: ObjectAdapter?) throws

    /// Gets the object adapter associated with this connection.
    ///
    /// - Returns: The object adapter associated with this connection.
    func getAdapter() -> ObjectAdapter?

    /// Gets the endpoint from which the connection was created.
    ///
    /// - Returns: The endpoint from which the connection was created.
    func getEndpoint() -> Endpoint

    /// Flushes any pending batch requests for this connection.
    /// This means all batch requests invoked on fixed proxies associated with the connection.
    ///
    /// - Parameter compress: Specifies whether or not the queued batch requests should be compressed
    /// before being sent over the wire.
    func flushBatchRequests(_ compress: CompressBatch) async throws

    /// Sets a close callback on the connection. The callback is called by the connection when it's closed. The
    /// callback is called from the Ice thread pool associated with the connection.
    /// If the callback needs more information about the closure, it can call ``throwException()``.
    ///
    /// - Parameter callback: The close callback object.
    func setCloseCallback(_ callback: CloseCallback?) throws

    /// Disables the inactivity check on this connection.
    func disableInactivityCheck()

    /// Returns the connection type. This corresponds to the endpoint type, such as "tcp", "udp", etc.
    ///
    /// - Returns: The type of the connection.
    func type() -> String

    /// Returns a description of the connection as human readable text, suitable for logging or error messages.
    /// This method remains usable after the connection is closed or aborted.
    ///
    /// - Returns: The description of the connection as human readable text.
    func toString() -> String

    /// Returns the connection information.
    ///
    /// - Returns: The connection information.
    func getInfo() throws -> ConnectionInfo

    /// Sets the size of the receive and send buffers.
    ///
    /// - Parameters:
    ///   - rcvSize: The size of the receive buffer.
    ///   - sndSize: The size of the send buffer.
    func setBufferSize(rcvSize: Int32, sndSize: Int32) throws

    /// Throws an exception that provides the reason for the closure of this connection. For example,
    /// this method throws ``CloseConnectionException`` when the connection was closed gracefully by the peer;
    /// it throws ``ConnectionAbortedException`` when the connection is aborted with ``abort()``.
    /// This method does nothing if the connection is not yet closed.
    func throwException() throws
}

/// Base class for all connection info classes.
open class ConnectionInfo {
    /// The information of the underlying transport or `nil` if there's no underlying transport.
    public let underlying: ConnectionInfo?

    /// Indicates whether the connection is an incoming connection.
    public let incoming: Bool

    /// The name of the adapter associated with the connection.
    public let adapterName: String

    /// The connection ID.
    public let connectionId: String

    public init(underlying: ConnectionInfo) {
        self.underlying = underlying
        self.incoming = underlying.incoming
        self.adapterName = underlying.adapterName
        self.connectionId = underlying.connectionId
    }

    public init(incoming: Bool, adapterName: String, connectionId: String) {
        self.underlying = nil
        self.incoming = incoming
        self.adapterName = adapterName
        self.connectionId = connectionId
    }
}

/// Provides access to the connection details of an IP connection.
open class IPConnectionInfo: ConnectionInfo {
    /// The local address.
    public let localAddress: String

    /// The local port.
    public let localPort: Int32

    /// The remote address.
    public let remoteAddress: String

    /// The remote port.
    public let remotePort: Int32

    public init(
        incoming: Bool, adapterName: String, connectionId: String, localAddress: String,
        localPort: Int32,
        remoteAddress: String, remotePort: Int32
    ) {
        self.localAddress = localAddress
        self.localPort = localPort
        self.remoteAddress = remoteAddress
        self.remotePort = remotePort
        super.init(incoming: incoming, adapterName: adapterName, connectionId: connectionId)
    }
}

/// Provides access to the connection details of a TCP connection.
public final class TCPConnectionInfo: IPConnectionInfo {
    /// The size of the receive buffer.
    public let rcvSize: Int32

    /// The size of the send buffer.
    public let sndSize: Int32

    internal init(
        incoming: Bool, adapterName: String, connectionId: String, localAddress: String,
        localPort: Int32,
        remoteAddress: String, remotePort: Int32, rcvSize: Int32, sndSize: Int32
    ) {
        self.rcvSize = rcvSize
        self.sndSize = sndSize
        super.init(
            incoming: incoming, adapterName: adapterName, connectionId: connectionId,
            localAddress: localAddress,
            localPort: localPort, remoteAddress: remoteAddress, remotePort: remotePort)
    }
}

/// Provides access to the connection details of an SSL connection.
public final class SSLConnectionInfo: ConnectionInfo {
    /// The certificate chain.
    public let peerCertificate: SecCertificate?

    internal init(underlying: ConnectionInfo, peerCertificate: SecCertificate?) {
        self.peerCertificate = peerCertificate
        super.init(underlying: underlying)
    }
}

/// Provides access to the connection details of a UDP connection.
public final class UDPConnectionInfo: IPConnectionInfo {
    /// The multicast address.
    public let mcastAddress: String

    /// The multicast port.
    public let mcastPort: Int32

    /// The size of the receive buffer.
    public let rcvSize: Int32

    /// The size of the send buffer.
    public let sndSize: Int32

    internal init(
        incoming: Bool, adapterName: String, connectionId: String, localAddress: String,
        localPort: Int32,
        remoteAddress: String, remotePort: Int32, mcastAddress: String, mcastPort: Int32,
        rcvSize: Int32, sndSize: Int32
    ) {
        self.mcastAddress = mcastAddress
        self.mcastPort = mcastPort
        self.rcvSize = rcvSize
        self.sndSize = sndSize
        super.init(
            incoming: incoming, adapterName: adapterName, connectionId: connectionId,
            localAddress: localAddress,
            localPort: localPort, remoteAddress: remoteAddress, remotePort: remotePort)
    }
}

/// Provides access to the connection details of a WebSocket connection.
public final class WSConnectionInfo: ConnectionInfo {
    /// The headers from the HTTP upgrade request.
    public let headers: HeaderDict

    internal init(underlying: ConnectionInfo, headers: HeaderDict) {
        self.headers = headers
        super.init(underlying: underlying)
    }
}

/// Provides access to the connection details of an IAP connection.
public final class IAPConnectionInfo: ConnectionInfo {
    /// The accessory name.
    public let name: String

    /// The accessory manufacturer.
    public let manufacturer: String

    /// The accessory model number.
    public let modelNumber: String

    /// The accessory firmware revision.
    public let firmwareRevision: String

    /// The accessory hardware revision.
    public let hardwareRevision: String

    /// The protocol used by the accessory.
    public let `protocol`: String

    internal init(
        incoming: Bool, adapterName: String, connectionId: String, name: String, manufacturer: String,
        modelNumber: String, firmwareRevision: String, hardwareRevision: String, `protocol`: String
    ) {
        self.name = name
        self.manufacturer = manufacturer
        self.modelNumber = modelNumber
        self.firmwareRevision = firmwareRevision
        self.hardwareRevision = hardwareRevision
        self.protocol = `protocol`
        super.init(incoming: incoming, adapterName: adapterName, connectionId: connectionId)
    }
}
