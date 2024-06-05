// Copyright (c) ZeroC, Inc.

import Foundation

/// The thread state enumeration keeps track of the different possible states of Ice threads.
public enum InstrumentationThreadState: UInt8 {
    /// ThreadStateIdle The thread is idle.
    case ThreadStateIdle = 0
    /// ThreadStateInUseForIO The thread is in use performing reads or writes for Ice connections. This state is only
    /// for threads from an Ice thread pool.
    case ThreadStateInUseForIO = 1
    /// ThreadStateInUseForUser The thread is calling user code (servant implementation, AMI callbacks). This state is
    /// only for threads from an Ice thread pool.
    case ThreadStateInUseForUser = 2
    /// ThreadStateInUseForOther The thread is performing other internal activities (DNS lookups, timer callbacks, etc).
    case ThreadStateInUseForOther = 3
    public init() {
        self = .ThreadStateIdle
    }
}

/// An `Ice.InputStream` extension to read `InstrumentationThreadState` enumerated values from the stream.
extension InputStream {
    /// Read an enumerated value.
    ///
    /// - returns: `InstrumentationThreadState` - The enumarated value.
    public func read() throws -> InstrumentationThreadState {
        let rawValue: UInt8 = try read(enumMaxValue: 3)
        guard let val = InstrumentationThreadState(rawValue: rawValue) else {
            throw MarshalException(reason: "invalid enum value")
        }
        return val
    }

    /// Read an optional enumerated value from the stream.
    ///
    /// - parameter tag: `Int32` - The numeric tag associated with the value.
    ///
    /// - returns: `InstrumentationThreadState` - The enumerated value.
    public func read(tag: Int32) throws -> InstrumentationThreadState? {
        guard try readOptional(tag: tag, expectedFormat: .Size) else {
            return nil
        }
        return try read() as InstrumentationThreadState
    }
}

/// An `Ice.OutputStream` extension to write `InstrumentationThreadState` enumerated values to the stream.
extension OutputStream {
    /// Writes an enumerated value to the stream.
    ///
    /// parameter _: `InstrumentationThreadState` - The enumerator to write.
    public func write(_ v: InstrumentationThreadState) {
        write(enum: v.rawValue, maxValue: 3)
    }

    /// Writes an optional enumerated value to the stream.
    ///
    /// parameter tag: `Int32` - The numeric tag associated with the value.
    ///
    /// parameter _: `InstrumentationThreadState` - The enumerator to write.
    public func write(tag: Int32, value: InstrumentationThreadState?) {
        guard let v = value else {
            return
        }
        write(tag: tag, val: v.rawValue, maxValue: 3)
    }
}

/// The state of an Ice connection.
public enum InstrumentationConnectionState: UInt8 {
    /// ConnectionStateValidating The connection is being validated.
    case ConnectionStateValidating = 0
    /// ConnectionStateHolding The connection is holding the reception of new messages.
    case ConnectionStateHolding = 1
    /// ConnectionStateActive The connection is active and can send and receive messages.
    case ConnectionStateActive = 2
    /// ConnectionStateClosing The connection is being gracefully shutdown and waits for the peer to close its end
    /// of the connection.
    case ConnectionStateClosing = 3
    /// ConnectionStateClosed The connection is closed and waits for potential dispatch to be finished before being
    /// destroyed and detached from the observer.
    case ConnectionStateClosed = 4
    public init() {
        self = .ConnectionStateValidating
    }
}

/// An `Ice.InputStream` extension to read `InstrumentationConnectionState` enumerated values from the stream.
extension InputStream {
    /// Read an enumerated value.
    ///
    /// - returns: `InstrumentationConnectionState` - The enumarated value.
    public func read() throws -> InstrumentationConnectionState {
        let rawValue: UInt8 = try read(enumMaxValue: 4)
        guard let val = InstrumentationConnectionState(rawValue: rawValue) else {
            throw MarshalException(reason: "invalid enum value")
        }
        return val
    }

    /// Read an optional enumerated value from the stream.
    ///
    /// - parameter tag: `Int32` - The numeric tag associated with the value.
    ///
    /// - returns: `InstrumentationConnectionState` - The enumerated value.
    public func read(tag: Int32) throws -> InstrumentationConnectionState? {
        guard try readOptional(tag: tag, expectedFormat: .Size) else {
            return nil
        }
        return try read() as InstrumentationConnectionState
    }
}

/// An `Ice.OutputStream` extension to write `InstrumentationConnectionState` enumerated values to the stream.
extension OutputStream {
    /// Writes an enumerated value to the stream.
    ///
    /// parameter _: `InstrumentationConnectionState` - The enumerator to write.
    public func write(_ v: InstrumentationConnectionState) {
        write(enum: v.rawValue, maxValue: 4)
    }

    /// Writes an optional enumerated value to the stream.
    ///
    /// parameter tag: `Int32` - The numeric tag associated with the value.
    ///
    /// parameter _: `InstrumentationConnectionState` - The enumerator to write.
    public func write(tag: Int32, value: InstrumentationConnectionState?) {
        guard let v = value else {
            return
        }
        write(tag: tag, val: v.rawValue, maxValue: 4)
    }
}

/// The object observer interface used by instrumented objects to notify the observer of their existence.
public protocol InstrumentationObserver: AnyObject {
    /// This method is called when the instrumented object is created or when the observer is attached to an existing
    /// object.
    func attach() throws

    /// This method is called when the instrumented object is destroyed and as a result the observer detached from the
    /// object.
    func detach() throws

    /// Notification of a failure.
    ///
    /// - parameter _: `String` The name of the exception.
    func failed(_ exceptionName: String) throws
}

/// The thread observer interface to instrument Ice threads. This can be threads from the Ice thread pool or utility
/// threads used by the Ice core.
public protocol InstrumentationThreadObserver: InstrumentationObserver {
    /// Notification of thread state change.
    ///
    /// - parameter oldState: `InstrumentationThreadState` The previous thread state.
    ///
    /// - parameter newState: `InstrumentationThreadState` The new thread state.
    func stateChanged(oldState: InstrumentationThreadState, newState: InstrumentationThreadState)
        throws
}

/// The connection observer interface to instrument Ice connections.
public protocol InstrumentationConnectionObserver: InstrumentationObserver {
    /// Notification of sent bytes over the connection.
    ///
    /// - parameter _: `Int32` The number of bytes sent.
    func sentBytes(_ num: Int32) throws

    /// Notification of received bytes over the connection.
    ///
    /// - parameter _: `Int32` The number of bytes received.
    func receivedBytes(_ num: Int32) throws
}

/// The dispatch observer to instrument servant dispatch.
public protocol InstrumentationDispatchObserver: InstrumentationObserver {
    /// Notification of a user exception.
    func userException() throws

    /// Reply notification.
    ///
    /// - parameter _: `Int32` The size of the reply.
    func reply(_ size: Int32) throws
}

/// The child invocation observer to instrument remote or collocated invocations.
public protocol InstrumentationChildInvocationObserver: InstrumentationObserver {
    /// Reply notification.
    ///
    /// - parameter _: `Int32` The size of the reply.
    func reply(_ size: Int32) throws
}

/// The remote observer to instrument invocations that are sent over the wire.
public protocol InstrumentationRemoteObserver: InstrumentationChildInvocationObserver {}

/// The collocated observer to instrument invocations that are collocated.
public protocol InstrumentationCollocatedObserver: InstrumentationChildInvocationObserver {}

/// The invocation observer to instrument invocations on proxies. A proxy invocation can either result in a collocated
/// or remote invocation. If it results in a remote invocation, a sub-observer is requested for the remote invocation.
public protocol InstrumentationInvocationObserver: InstrumentationObserver {
    /// Notification of the invocation being retried.
    func retried() throws

    /// Notification of a user exception.
    func userException() throws

    /// Get a remote observer for this invocation.
    ///
    /// - parameter con: `ConnectionInfo?` The connection information.
    ///
    /// - parameter endpt: `Endpoint?` The connection endpoint.
    ///
    /// - parameter requestId: `Int32` The ID of the invocation.
    ///
    /// - parameter size: `Int32` The size of the invocation.
    ///
    /// - returns: `InstrumentationRemoteObserver?` - The observer to instrument the remote invocation.
    func getRemoteObserver(
        con: ConnectionInfo?, endpt: Endpoint?, requestId: Int32, size: Int32
    )
        throws -> InstrumentationRemoteObserver?

    /// Get a collocated observer for this invocation.
    ///
    /// - parameter adapter: `ObjectAdapter?` The object adapter hosting the collocated Ice object.
    ///
    /// - parameter requestId: `Int32` The ID of the invocation.
    ///
    /// - parameter size: `Int32` The size of the invocation.
    ///
    /// - returns: `InstrumentationCollocatedObserver?` - The observer to instrument the collocated invocation.
    func getCollocatedObserver(adapter: ObjectAdapter?, requestId: Int32, size: Int32)
        throws -> InstrumentationCollocatedObserver?
}

/// The observer updater interface. This interface is implemented by the Ice run-time and an instance of this interface
/// is provided by the Ice communicator on initialization to the CommunicatorObserver object set with the
/// communicator initialization data. The Ice communicator calls CommunicatorObserver.setObserverUpdater to
/// provide the observer updater.
/// This interface can be used by add-ins implementing the CommunicatorObserver interface to update the
/// observers of connections and threads.
public protocol InstrumentationObserverUpdater: AnyObject {
    /// Update connection observers associated with each of the Ice connection from the communicator and its object
    /// adapters.
    /// When called, this method goes through all the connections and for each connection
    /// CommunicatorObserver.getConnectionObserver is called. The implementation of getConnectionObserver has
    /// the possibility to return an updated observer if necessary.
    func updateConnectionObservers() throws

    /// Update thread observers associated with each of the Ice thread from the communicator and its object adapters.
    /// When called, this method goes through all the threads and for each thread
    /// CommunicatorObserver.getThreadObserver is called. The implementation of getThreadObserver has the
    /// possibility to return an updated observer if necessary.
    func updateThreadObservers() throws
}

/// The communicator observer interface used by the Ice run-time to obtain and update observers for its observable
/// objects. This interface should be implemented by add-ins that wish to observe Ice objects in order to collect
/// statistics. An instance of this interface can be provided to the Ice run-time through the Ice communicator
/// initialization data.
public protocol InstrumentationCommunicatorObserver: AnyObject {
    /// This method should return an observer for the given endpoint information and connector. The Ice run-time calls
    /// this method for each connection establishment attempt.
    ///
    /// - parameter endpt: `Endpoint?` The endpoint.
    ///
    /// - parameter connector: `String` The description of the connector. For IP transports, this is typically
    /// the IP address to connect to.
    ///
    /// - returns: `InstrumentationObserver?` - The observer to instrument the connection establishment.
    func getConnectionEstablishmentObserver(endpt: Endpoint?, connector: String)
        throws -> InstrumentationObserver?

    /// This method should return an observer for the given endpoint information. The Ice run-time calls this method to
    /// resolve an endpoint and obtain the list of connectors. For IP endpoints, this typically involves doing a DNS
    /// lookup to obtain the IP addresses associated with the DNS name.
    ///
    /// - parameter _: `Endpoint?` The endpoint.
    ///
    /// - returns: `InstrumentationObserver?` - The observer to instrument the endpoint lookup.
    func getEndpointLookupObserver(_ endpt: Endpoint?) throws -> InstrumentationObserver?

    /// This method should return a connection observer for the given connection. The Ice run-time calls this method
    /// for each new connection and for all the Ice communicator connections when
    /// ObserverUpdater.updateConnectionObservers is called.
    ///
    /// - parameter c: `ConnectionInfo?` The connection information.
    ///
    /// - parameter e: `Endpoint?` The connection endpoint.
    ///
    /// - parameter s: `InstrumentationConnectionState` The state of the connection.
    ///
    /// - parameter o: `InstrumentationConnectionObserver?` The old connection observer if one is already set
    /// or a null reference otherwise.
    ///
    /// - returns: `InstrumentationConnectionObserver?` - The connection observer to instrument the connection.
    func getConnectionObserver(
        c: ConnectionInfo?,
        e: Endpoint?,
        s: InstrumentationConnectionState,
        o: InstrumentationConnectionObserver?
    ) throws -> InstrumentationConnectionObserver?

    /// This method should return a thread observer for the given thread. The Ice run-time calls this method for each
    /// new thread and for all the Ice communicator threads when ObserverUpdater.updateThreadObservers is
    /// called.
    ///
    /// - parameter parent: `String` The parent of the thread.
    ///
    /// - parameter id: `String` The ID of the thread to observe.
    ///
    /// - parameter s: `InstrumentationThreadState` The state of the thread.
    ///
    /// - parameter o: `InstrumentationThreadObserver?` The old thread observer if one is already set or a null
    /// reference otherwise.
    ///
    /// - returns: `InstrumentationThreadObserver?` - The thread observer to instrument the thread.
    func getThreadObserver(
        parent: String,
        id: String,
        s: InstrumentationThreadState,
        o: InstrumentationThreadObserver?
    ) throws -> InstrumentationThreadObserver?

    /// This method should return an invocation observer for the given invocation. The Ice run-time calls this method
    /// for each new invocation on a proxy.
    ///
    /// - parameter prx: `ObjectPrx?` The proxy used for the invocation.
    ///
    /// - parameter operation: `String` The name of the operation.
    ///
    /// - parameter ctx: `Context` The context specified by the user.
    ///
    /// - returns: `InstrumentationInvocationObserver?` - The invocation observer to instrument the invocation.
    func getInvocationObserver(
        prx: ObjectPrx?,
        operation: String,
        ctx: Context
    ) throws -> InstrumentationInvocationObserver?

    /// This method should return a dispatch observer for the given dispatch. The Ice run-time calls this method each
    /// time it receives an incoming invocation to be dispatched for an Ice object.
    ///
    /// - parameter c: `Current` The current object as provided to the Ice servant dispatching the invocation.
    ///
    /// - parameter size: `Int32` The size of the dispatch.
    ///
    /// - returns: `InstrumentationDispatchObserver?` - The dispatch observer to instrument the dispatch.
    func getDispatchObserver(c: Current, size: Int32) throws -> InstrumentationDispatchObserver?

    /// The Ice run-time calls this method when the communicator is initialized. The add-in implementing this
    /// interface can use this object to get the Ice run-time to re-obtain observers for observed objects.
    ///
    /// - parameter _: `InstrumentationObserverUpdater?` The observer updater object.
    func setObserverUpdater(_ updater: InstrumentationObserverUpdater?) throws
}
