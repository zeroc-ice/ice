//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * This exception is raised when a failure occurs during initialization.
     */
    class InitializationException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
        reason:string;
    }

    /**
     * This exception indicates that a failure occurred while initializing
     * a plug-in.
     */
    class PluginInitializationException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
        reason:string;
    }

    /**
     * This exception is raised if a feature is requested that is not
     * supported with collocation optimization.
     *
     * @deprecated This exception is no longer used by the Ice run time
     */
    class CollocationOptimizationException extends LocalException
    {
    }

    /**
     * An attempt was made to register something more than once with
     * the Ice run time.
     *
     * This exception is raised if an attempt is made to register a
     * servant, servant locator, facet, value factory, plug-in, object
     * adapter, object, or user exception factory more than once for the
     * same ID.
     */
    class AlreadyRegisteredException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param kindOfObject The kind of object that could not be removed: "servant", "facet", "object", "default servant", "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
         * @param id The ID (or name) of the object that is registered already.
         * @param ice_cause The error that cause this exception.
         */
        constructor(kindOfObject?:string, id?:string, ice_cause?:string|Error);
        kindOfObject:string;
        id:string;
    }

    /**
     * An attempt was made to find or deregister something that is not
     * registered with the Ice run time or Ice locator.
     *
     * This exception is raised if an attempt is made to remove a servant,
     * servant locator, facet, value factory, plug-in, object adapter,
     * object, or user exception factory that is not currently registered.
     *
     * It's also raised if the Ice locator can't find an object or object
     * adapter when resolving an indirect proxy or when an object adapter
     * is activated.
     */
    class NotRegisteredException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param kindOfObject The kind of object that could not be removed: "servant", "facet", "object", "default servant", "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
         * @param id The ID (or name) of the object that could not be removed.
         * @param ice_cause The error that cause this exception.
         */
        constructor(kindOfObject?:string, id?:string, ice_cause?:string|Error);
        kindOfObject:string;
        id:string;
    }

    /**
     * The operation can only be invoked with a twoway request.
     *
     * This exception is raised if an attempt is made to invoke an
     * operation with <code>ice_oneway</code>, <code>ice_batchOneway</code>, <code>ice_datagram</code>,
     * or <code>ice_batchDatagram</code> and the operation has a return value,
     * out-parameters, or an exception specification.
     */
    class TwowayOnlyException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param operation The name of the operation that was invoked.
         * @param ice_cause The error that cause this exception.
         */
        constructor(operation?:string, ice_cause?:string|Error);
        operation:string;
    }

    /**
     * An attempt was made to clone a class that does not support
     * cloning.
     *
     * This exception is raised if <code>ice_clone</code> is called on
     * a class that is derived from an abstract Slice class (that is,
     * a class containing operations), and the derived class does not
     * provide an implementation of the <code>ice_clone</code> operation (C++ only).
     */
    class CloneNotImplementedException extends LocalException
    {
    }

    /**
     * This exception is raised if an operation call on a server raises an
     * unknown exception. For example, for C++, this exception is raised
     * if the server throws a C++ exception that is not directly or
     * indirectly derived from <code>Ice::LocalException</code> or
     * <code>Ice::UserException</code>.
     */
    class UnknownException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param unknown This field is set to the textual representation of the unknown exception if available.
         * @param ice_cause The error that cause this exception.
         */
        constructor(unknown?:string, ice_cause?:string|Error);
        unknown:string;
    }

    /**
     * This exception is raised if an operation call on a server raises a
     * local exception. Because local exceptions are not transmitted by
     * the Ice protocol, the client receives all local exceptions raised
     * by the server as {@link UnknownLocalException}. The only exception to this
     * rule are all exceptions derived from {@link RequestFailedException},
     * which are transmitted by the Ice protocol even though they are
     * declared <code>local</code>.
     */
    class UnknownLocalException extends UnknownException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param unknown This field is set to the textual representation of the unknown exception if available.
         * @param ice_cause The error that cause this exception.
         */
        constructor(unknown?:string, ice_cause?:string|Error);
    }

    /**
     * An operation raised an incorrect user exception.
     *
     * This exception is raised if an operation raises a
     * user exception that is not declared in the exception's
     * <code>throws</code> clause. Such undeclared exceptions are
     * not transmitted from the server to the client by the Ice
     * protocol, but instead the client just gets an
     * {@link UnknownUserException}. This is necessary in order to not violate
     * the contract established by an operation's signature: Only local
     * exceptions and user exceptions declared in the
     * <code>throws</code> clause can be raised.
     */
    class UnknownUserException extends UnknownException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param unknown This field is set to the textual representation of the unknown exception if available.
         * @param ice_cause The error that cause this exception.
         */
        constructor(unknown?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if the Ice library version does not match
     * the version in the Ice header files.
     */
    class VersionMismatchException extends LocalException
    {
    }

    /**
     * This exception is raised if the {@link Communicator} has been destroyed.
     * @see Communicator#destroy
     */
    class CommunicatorDestroyedException extends LocalException
    {
    }

    /**
     * This exception is raised if an attempt is made to use a deactivated
     * {@link ObjectAdapter}.
     * @see ObjectAdapter#deactivate
     * @see Communicator#shutdown
     */
    class ObjectAdapterDeactivatedException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param name Name of the adapter.
         * @param ice_cause The error that cause this exception.
         */
        constructor(name?:string, ice_cause?:string|Error);
        name:string;
    }

    /**
     * This exception is raised if an {@link ObjectAdapter} cannot be activated.
     *
     * This happens if the {@link Locator} detects another active {@link ObjectAdapter} with
     * the same adapter id.
     */
    class ObjectAdapterIdInUseException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param id Adapter ID.
         * @param ice_cause The error that cause this exception.
         */
        constructor(id?:string, ice_cause?:string|Error);
        id:string;
    }

    /**
     * This exception is raised if no suitable endpoint is available.
     */
    class NoEndpointException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param proxy The stringified proxy for which no suitable endpoint is available.
         * @param ice_cause The error that cause this exception.
         */
        constructor(proxy?:string, ice_cause?:string|Error);
        proxy:string;
    }

    /**
     * This exception is raised if there was an error while parsing an
     * endpoint.
     */
    class EndpointParseException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param str Describes the failure and includes the string that could not be parsed.
         * @param ice_cause The error that cause this exception.
         */
        constructor(str?:string, ice_cause?:string|Error);
        str:string;
    }

    /**
     * This exception is raised if there was an error while parsing an
     * endpoint selection type.
     */
    class EndpointSelectionTypeParseException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param str Describes the failure and includes the string that could not be parsed.
         * @param ice_cause The error that cause this exception.
         */
        constructor(str?:string, ice_cause?:string|Error);
        str:string;
    }

    /**
     * This exception is raised if there was an error while parsing a
     * version.
     */
    class VersionParseException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param str Describes the failure and includes the string that could not be parsed.
         * @param ice_cause The error that cause this exception.
         */
        constructor(str?:string, ice_cause?:string|Error);
        str:string;
    }

    /**
     * This exception is raised if there was an error while parsing a
     * stringified identity.
     */
    class IdentityParseException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param str Describes the failure and includes the string that could not be parsed.
         * @param ice_cause The error that cause this exception.
         */
        constructor(str?:string, ice_cause?:string|Error);
        str:string;
    }

    /**
     * This exception is raised if there was an error while parsing a
     * stringified proxy.
     */
    class ProxyParseException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param str Describes the failure and includes the string that could not be parsed.
         * @param ice_cause The error that cause this exception.
         */
        constructor(str?:string, ice_cause?:string|Error);
        str:string;
    }

    /**
     * This exception is raised if an illegal identity is encountered.
     */
    class IllegalIdentityException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param id The illegal identity.
         * @param ice_cause The error that cause this exception.
         */
        constructor(id?:Identity, ice_cause?:string|Error);
        id:Identity;
    }

    /**
     * This exception is raised to reject an illegal servant (typically
     * a null servant)
     */
    class IllegalServantException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason Describes why this servant is illegal.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
        reason:string;
    }

    /**
     * This exception is raised if a request failed. This exception, and
     * all exceptions derived from {@link RequestFailedException}, are
     * transmitted by the Ice protocol, even though they are declared
     * <code>local</code>.
     */
    class RequestFailedException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         * @param ice_cause The error that cause this exception.
         */
        constructor(id?:Identity, facet?:string, operation?:string, ice_cause?:string|Error);
        id:Identity;
        facet:string;
        operation:string;
    }

    /**
     * This exception is raised if an object does not exist on the server,
     * that is, if no facets with the given identity exist.
     */
    class ObjectNotExistException extends RequestFailedException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         * @param ice_cause The error that cause this exception.
         */
        constructor(id?:Identity, facet?:string, operation?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if no facet with the given name exists,
     * but at least one facet with the given identity exists.
     */
    class FacetNotExistException extends RequestFailedException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         * @param ice_cause The error that cause this exception.
         */
        constructor(id?:Identity, facet?:string, operation?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if an operation for a given object does
     * not exist on the server. Typically this is caused by either the
     * client or the server using an outdated Slice specification.
     */
    class OperationNotExistException extends RequestFailedException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param id The identity of the Ice Object to which the request was sent.
         * @param facet The facet to which the request was sent.
         * @param operation The operation name of the request.
         * @param ice_cause The error that cause this exception.
         */
        constructor(id?:Identity, facet?:string, operation?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if a system error occurred in the server
     * or client process. There are many possible causes for such a system
     * exception. For details on the cause, {@link SyscallException#error}
     * should be inspected.
     */
    class SyscallException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param error The error number describing the system exception.
         * @param ice_cause The error that cause this exception.
         */
        constructor(error?:number, ice_cause?:string|Error);
        error:number;
    }

    /**
     * This exception indicates socket errors.
     */
    class SocketException extends SyscallException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param error The error number describing the system exception.
         * @param ice_cause The error that cause this exception.
         */
        constructor(error?:number, ice_cause?:string|Error);
    }

    /**
     * This exception indicates CFNetwork errors.
     */
    class CFNetworkException extends SocketException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param error The error number describing the system exception.
         * @param domain The domain of the error.
         * @param ice_cause The error that cause this exception.
         */
        constructor(error?:number, domain?:string, ice_cause?:string|Error);
        domain:string;
    }

    /**
     * This exception indicates file errors.
     */
    class FileException extends SyscallException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param error The error number describing the system exception.
         * @param path The path of the file responsible for the error.
         * @param ice_cause The error that cause this exception.
         */
        constructor(error?:number, path?:string, ice_cause?:string|Error);
        path:string;
    }

    /**
     * This exception indicates connection failures.
     */
    class ConnectFailedException extends SocketException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param error The error number describing the system exception.
         * @param ice_cause The error that cause this exception.
         */
        constructor(error?:number, ice_cause?:string|Error);
    }

    /**
     * This exception indicates a connection failure for which
     * the server host actively refuses a connection.
     */
    class ConnectionRefusedException extends ConnectFailedException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param error The error number describing the system exception.
         * @param ice_cause The error that cause this exception.
         */
        constructor(error?:number, ice_cause?:string|Error);
    }

    /**
     * This exception indicates a lost connection.
     */
    class ConnectionLostException extends SocketException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param error The error number describing the system exception.
         * @param ice_cause The error that cause this exception.
         */
        constructor(error?:number, ice_cause?:string|Error);
    }

    /**
     * This exception indicates a DNS problem. For details on the cause,
     * {@link DNSException#error} should be inspected.
     */
    class DNSException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param error The error number describing the DNS problem.
         * @param host The host name that could not be resolved.
         * @param ice_cause The error that cause this exception.
         */
        constructor(error?:number, host?:string, ice_cause?:string|Error);
        error:number;
        host:string;
    }

    /**
     * This exception indicates a request was interrupted.
     */
    class OperationInterruptedException extends LocalException
    {
    }

    /**
     * This exception indicates a timeout condition.
     */
    class TimeoutException extends LocalException
    {
    }

    /**
     * This exception indicates a connection establishment timeout condition.
     */
    class ConnectTimeoutException extends TimeoutException
    {
    }

    /**
     * This exception indicates a connection closure timeout condition.
     */
    class CloseTimeoutException extends TimeoutException
    {
    }

    /**
     * This exception indicates that a connection has been shut down because it has been
     * idle for some time.
     */
    class ConnectionTimeoutException extends TimeoutException
    {
    }

    /**
     * This exception indicates that an invocation failed because it timed
     * out.
     */
    class InvocationTimeoutException extends TimeoutException
    {
    }

    /**
     * This exception indicates that an asynchronous invocation failed
     * because it was canceled explicitly by the user.
     */
    class InvocationCanceledException extends LocalException
    {
    }

    /**
     * A generic exception base for all kinds of protocol error
     * conditions.
     */
    class ProtocolException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
        reason:string;
    }

    /**
     * This exception indicates that a message did not start with the expected
     * magic number ('I', 'c', 'e', 'P').
     */
    class BadMagicException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param badMagic A sequence containing the first four bytes of the incorrect message.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, badMagic?:ByteSeq, ice_cause?:string|Error);
        badMagic:ByteSeq;
    }

    /**
     * This exception indicates an unsupported protocol version.
     */
    class UnsupportedProtocolException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param bad The version of the unsupported protocol.
         * @param supported The version of the protocol that is supported.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, bad?:ProtocolVersion, supported?:ProtocolVersion, ice_cause?:string|Error);
        bad:ProtocolVersion;
        supported:ProtocolVersion;
    }

    /**
     * This exception indicates an unsupported data encoding version.
     */
    class UnsupportedEncodingException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param bad The version of the unsupported encoding.
         * @param supported The version of the encoding that is supported.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, bad?:EncodingVersion, supported?:EncodingVersion, ice_cause?:string|Error);
        bad:EncodingVersion;
        supported:EncodingVersion;
    }

    /**
     * This exception indicates that an unknown protocol message has been received.
     */
    class UnknownMessageException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if a message is received over a connection
     * that is not yet validated.
     */
    class ConnectionNotValidatedException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception indicates that a response for an unknown request ID has been
     * received.
     */
    class UnknownRequestIdException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception indicates that an unknown reply status has been received.
     */
    class UnknownReplyStatusException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception indicates that the connection has been gracefully shut down by the
     * server. The operation call that caused this exception has not been
     * executed by the server. In most cases you will not get this
     * exception, because the client will automatically retry the
     * operation call in case the server shut down the connection. However,
     * if upon retry the server shuts down the connection again, and the
     * retry limit has been reached, then this exception is propagated to
     * the application code.
     */
    class CloseConnectionException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised by an operation call if the application
     * closes the connection locally using {@link Connection#close}.
     * @see Connection#close
     */
    class ConnectionManuallyClosedException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param graceful True if the connection was closed gracefully, false otherwise.
         * @param ice_cause The error that cause this exception.
         */
        constructor(graceful?:boolean, ice_cause?:string|Error);
        graceful:boolean;
    }

    /**
     * This exception indicates that a message size is less
     * than the minimum required size.
     */
    class IllegalMessageSizeException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception indicates a problem with compressing or uncompressing data.
     */
    class CompressionException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * A datagram exceeds the configured size.
     *
     * This exception is raised if a datagram exceeds the configured send or receive buffer
     * size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
     */
    class DatagramLimitException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised for errors during marshaling or unmarshaling data.
     */
    class MarshalException extends ProtocolException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if inconsistent data is received while unmarshaling a proxy.
     */
    class ProxyUnmarshalException extends MarshalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if an out-of-bounds condition occurs during unmarshaling.
     */
    class UnmarshalOutOfBoundsException extends MarshalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if no suitable value factory was found during
     * unmarshaling of a Slice class instance.
     * @see ValueFactory
     * @see Communicator#getValueFactoryManager
     * @see ValueFactoryManager#add
     * @see ValueFactoryManager#find
     */
    class NoValueFactoryException extends MarshalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param type The Slice type ID of the class instance for which no no factory could be found.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, type?:string, ice_cause?:string|Error);
        type:string;
    }

    /**
     * This exception is raised if the type of an unmarshaled Slice class instance does
     * not match its expected type.
     * This can happen if client and server are compiled with mismatched Slice
     * definitions or if a class of the wrong type is passed as a parameter
     * or return value using dynamic invocation. This exception can also be
     * raised if IceStorm is used to send Slice class instances and
     * an operation is subscribed to the wrong topic.
     */
    class UnexpectedObjectException extends MarshalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param type The Slice type ID of the class instance that was unmarshaled.
         * @param expectedType The Slice type ID that was expected by the receiving operation.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, type?:string, expectedType?:string, ice_cause?:string|Error);
        type:string;
        expectedType:string;
    }

    /**
     * This exception is raised when Ice receives a request or reply
     * message whose size exceeds the limit specified by the
     * <code>Ice.MessageSizeMax</code> property.
     */
    class MemoryLimitException extends MarshalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised when a string conversion to or from UTF-8
     * fails during marshaling or unmarshaling.
     */
    class StringConversionException extends MarshalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception indicates a malformed data encapsulation.
     */
    class EncapsulationException extends MarshalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
    }

    /**
     * This exception is raised if an unsupported feature is used. The
     * unsupported feature string contains the name of the unsupported
     * feature
     */
    class FeatureNotSupportedException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param unsupportedFeature The name of the unsupported feature.
         * @param ice_cause The error that cause this exception.
         */
        constructor(unsupportedFeature?:string, ice_cause?:string|Error);
        unsupportedFeature:string;
    }

    /**
     * This exception indicates a failure in a security subsystem,
     * such as the IceSSL plug-in.
     */
    class SecurityException extends LocalException
    {
        /**
         * One-shot constructor to initialize all data members.
         * @param reason The reason for the failure.
         * @param ice_cause The error that cause this exception.
         */
        constructor(reason?:string, ice_cause?:string|Error);
        reason:string;
    }

    /**
     * This exception indicates that an attempt has been made to
     * change the connection properties of a fixed proxy.
     */
    class FixedProxyException extends LocalException
    {
    }

    /**
     * Indicates that the response to a request has already been sent;
     * re-dispatching such a request is not possible.
     */
    class ResponseSentException extends LocalException
    {
    }
}
