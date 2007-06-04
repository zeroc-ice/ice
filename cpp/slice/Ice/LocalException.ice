// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_LOCAL_EXCEPTION_ICE
#define ICE_LOCAL_EXCEPTION_ICE

#include <Ice/Identity.ice>
#include <Ice/BuiltinSequences.ice>

module Ice
{

/**
 *
 * This exception is raised when a failure occurs during initialization.
 *
 **/
local exception InitializationException
{
    /**
     *
     * The reason for the failure.
     *
     **/
    string reason;
};

/**
 *
 * This exception indicates that a failure occurred while initializing
 * a plug-in.
 *
 **/
local exception PluginInitializationException
{
    /**
     *
     * The reason for the failure.
     *
     **/
    string reason;
};

/**
 *
 * This exception is raised if a feature is requested that is not
 * supported with collocation optimization.
 *
 **/
local exception CollocationOptimizationException
{
};

/**
 *
 * An attempt was made to register something more than once with
 * the Ice run time.
 *
 * This exception is raised if an attempt is made to register a
 * servant, servant locator, facet, object factory, plug-in, object
 * adapter, object, or user exception factory more than once for the
 * same ID.
 *
 **/
local exception AlreadyRegisteredException
{
    /**
     *
     * The kind of object that is registered already: "servant",
     * "servant locator", "facet", "object factory", "plug-in",
     * "object adapter", "object", or "user exception factory".
     *
     **/
    string kindOfObject;

    /**
     *
     * The ID (or name) of the object that is registered already.
     *
     **/
    string id;
};

/**
 *
 * An attempt was made to deregister something that is not
 * registered with the Ice run time.
 *
 * This exception is raised if an attempt is made to remove a servant,
 * facet, object factory, plug-in, object adapter, object, or user
 * exception factory that is not currently registered.
 *
 **/
local exception NotRegisteredException
{
    /**
     *
     * The kind of object that could not be removed: "servant",
     * "facet", "object factory", "plug-in", "object adapter",
     * "object", or "user exception factory".
     *
     **/
    string kindOfObject;

    /**
     *
     * The ID (or name) of the object that could not be removed.
     *
     **/
    string id;
};

/**
 *
 * The operation can only be invoked with a twoway request.
 *
 * This exception is raised if an attempt is made to invoke an
 * operation with [ice_oneway], [ice_batchOneway], [ice_datagram],
 * or [ice_batchDatagram] and the operation has a return value,
 * out-parameters, or an exception specification.
 *
 **/
local exception TwowayOnlyException
{
    /**
     *
     * The name of the operation that was invoked.
     *
     **/
    string operation;
};

/**
 *
 * An attempt was made to clone a class that does not support
 * cloning.
 *
 * This exception is raised if [ice_clone] is called on
 * a class that is derived from an abstract Slice class (that is,
 * a class containing operations), and the derived class does not
 * provide an implementation of the [ice_clone] operation (C++ only).
 *
 **/
local exception CloneNotImplementedException
{
};

/**
 *
 * This exception is raised if an operation call on a server raises an
 * unknown exception. For example, for C++, this exception is raised
 * if the server throws a C++ exception that is not directly or
 * indirectly derived from <tt>Ice::LocalException</tt> or
 * <tt>Ice::UserException</tt>.
 *
 **/
local exception UnknownException
{
    /**
     *
     * A textual representation of the unknown exception. This field may
     * or may not be set, depending on the security policy of the
     * server. Some servers may give this information to clients for
     * debugging purposes, while others may not wish to disclose
     * information about server internals.
     *
     **/
    string unknown;
};
    
/**
 *
 * This exception is raised if an operation call on a server raises a
 * local exception. Because local exceptions are not transmitted by
 * the Ice protocol, the client receives all local exceptions raised
 * by the server as [UnknownLocalException]. The only exception to this
 * rule are all exceptions derived from [RequestFailedException],
 * which are transmitted by the Ice protocol even though they are
 * declared [local].
 *
 **/
local exception UnknownLocalException extends UnknownException
{
};

/**
 *
 * An operation raised an incorrect user exception.
 *
 * This exception is raised if an operation raises a
 * user exception that is not declared in the exception's
 * <tt>throws</tt> clause. Such undeclared exceptions are
 * not transmitted from the server to the client by the Ice
 * protocol, but instead the client just gets an
 * [UnknownUserException]. This is necessary in order to not violate
 * the contract established by an operation's signature: Only local
 * exceptions and user exceptions declared in the
 * <tt>throws</tt> clause can be raised.
 *
 **/
local exception UnknownUserException extends UnknownException
{
};

/**
 *
 * This exception is raised if the Ice library version does not match
 * the version in the Ice header files.
 *
 **/
local exception VersionMismatchException
{
};

/**
 *
 * This exception is raised if the [Communicator] has been destroyed.
 *
 * @see Communicator::destroy
 *
 **/
local exception CommunicatorDestroyedException
{
};

/**
 *
 * This exception is raised if an attempt is made to use a deactivated
 * [ObjectAdapter].
 *
 * @see ObjectAdapter::deactivate
 * @see Communicator::shutdown
 *
 **/
local exception ObjectAdapterDeactivatedException
{
    /**
     * 
     * Name of the adapter.
     *
     **/
    string name;
};

/**
 *
 * This exception is raised if an [ObjectAdapter] cannot be activated.
 *
 * This happens if the [Locator] detects another active [ObjectAdapter] with
 * the same adapter id.
 *
 **/
local exception ObjectAdapterIdInUseException
{
    /**
     *
     * Adapter ID.
     *
     **/
    string id;
};

/**
 *
 * This exception is raised if no suitable endpoint is available.
 *
 **/
local exception NoEndpointException
{
    /**
     * 
     * The stringified proxy for which no suitable endpoint is
     * available.
     *
     **/
    string proxy;
};

/**
 *
 * This exception is raised if there was an error while parsing an
 * endpoint.
 *
 **/
local exception EndpointParseException
{
    /**
     *
     * The string that could not be parsed.
     *
     **/
    string str;
};

/**
 *
 * This exception is raised if there was an error while parsing an
 * endpoint selection type.
 *
 **/
local exception EndpointSelectionTypeParseException
{
    /**
     *
     * The string that could not be parsed.
     *
     **/
    string str;
};

/**
 *
 * This exception is raised if there was an error while parsing a
 * stringified identity.
 *
 **/
local exception IdentityParseException
{
    /**
     *
     * The string that could not be parsed.
     *
     **/
    string str;
};

/**
 *
 * This exception is raised if there was an error while parsing a
 * stringified proxy.
 *
 **/
local exception ProxyParseException
{
    /**
     *
     * The string that could not be parsed.
     *
     **/
    string str;
};

/**
 *
 * This exception is raised if an illegal identity is encountered.
 *
 **/
local exception IllegalIdentityException
{
    /**
     *
     * The illegal identity.
     *
     **/
    Identity id;
};

/**
 *
 * This exception is raised if a request failed. This exception, and
 * all exceptions derived from [RequestFailedException], are
 * transmitted by the Ice protocol, even though they are declared
 * [local].
 *
 **/
local exception RequestFailedException
{
    /** The identity of the Ice Object to which the request was sent. */
    Identity id;

    /** The facet to which the request was sent. */
    string facet;

    /** The operation name of the request. */
    string operation;
};

/**
 *
 * This exception is raised if an object does not exist on the server,
 * that is, if no facets with the given identity exist.
 *
 **/
local exception ObjectNotExistException extends RequestFailedException
{
};

/**
 *
 * This exception is raised if no facet with the given name exists,
 * but at least one facet with the given identity exists.
 *
 **/
local exception FacetNotExistException extends RequestFailedException
{
};

/**
 *
 * This exception is raised if an operation for a given object does
 * not exist on the server. Typically this is caused by either the
 * client or the server using an outdated Slice specification.
 *
 **/
local exception OperationNotExistException extends RequestFailedException
{
};

/**
 *
 * This exception is raised if a system error occurred in the server
 * or client process. There are many possible causes for such a system
 * exception. For details on the cause, [SyscallException::error]
 * should be inspected.
 *
 **/
local exception SyscallException
{
    /**
     *
     * The error number describing the system exception. For C++ and
     * Unix, this is equivalent to <tt>errno</tt>. For C++
     * and Windows, this is the value returned by
     * <tt>GetLastError()</tt> or
     * <tt>WSAGetLastError()</tt>.
     *
     **/
    int error; // Don't use errno, as errno is usually a macro.
};

/**
 *
 * This exception indicates socket errors.
 *
 **/
local exception SocketException extends SyscallException
{
};

/**
 *
 * This exception indicates file errors.
 *
 **/
local exception FileException extends SyscallException
{
    /** The path of the file responsible for the error. */
    string path;
};

/**
 *
 * This exception indicates connection failures.
 *
 **/
local exception ConnectFailedException extends SocketException
{
};

/**
 *
 * This exception indicates a connection failure for which
 * the server host actively refuses a connection.
 *
 **/
local exception ConnectionRefusedException extends ConnectFailedException
{
};

/**
 *
 * This exception indicates a lost connection.
 *
 **/
local exception ConnectionLostException extends SocketException
{
};

/**
 *
 * This exception indicates a DNS problem. For details on the cause,
 * [DNSException::error] should be inspected.
 *
 **/
local exception DNSException
{
    /**
     *
     * The error number describing the DNS problem. For C++ and Unix,
     * this is equivalent to <tt>h_errno</tt>. For C++ and
     * Windows, this is the value returned by
     * <tt>WSAGetLastError()</tt>.
     *
     **/
    int error; // Don't use h_errno, as h_errno is usually a macro.

    /**
     *
     * The host name that could not be resolved.
     *
     **/
    string host;
};

/**
 *
 * This exception indicates a timeout condition.
 *
 **/
local exception TimeoutException
{
};

/**
 *
 * This exception indicates a connection establishment timeout condition.
 *
 **/
local exception ConnectTimeoutException extends TimeoutException
{
};

/**
 *
 * This exception indicates a connection closure timeout condition.
 *
 **/
local exception CloseTimeoutException extends TimeoutException
{
};

/**
 *
 * This exception indicates that a connection has been shut down because it has been
 * idle for some time.
 *
 **/
local exception ConnectionTimeoutException extends TimeoutException
{
};

/**
 *
 * A generic exception base for all kinds of protocol error
 * conditions.
 *
 **/
local exception ProtocolException
{
    /**
     *
     * The reason for the failure.
     *
     **/
    string reason;
};

/**
 *
 * This exception indicates that a message did not start with the expected
 * magic number ('I', 'c', 'e', 'P').
 *
 **/
local exception BadMagicException extends ProtocolException
{
    /**
     *
     * A sequence containing the first four bytes of the incorrect message.
     *
     **/
    ByteSeq badMagic;
};

/**
 *
 * This exception indicates an unsupported protocol version.
 *
 **/
local exception UnsupportedProtocolException extends ProtocolException
{
    /**
     *
     * The major version number of the unsupported protocol.
     *
     **/
    int badMajor;

    /**
     *
     * The minor version number of the unsupported protocol.
     *
     **/
    int badMinor;

    /**
     *
     * The major version number of the protocol that is supported.
     *
     **/
    int major;

    /**
     *
     * The highest minor version number of the protocol that can be supported.
     *
     **/
    int minor;
};

/**
 *
 * This exception indicates an unsupported data encoding version.
 *
 **/
local exception UnsupportedEncodingException extends ProtocolException
{
    /**
     *
     * The major version number of the unsupported encoding.
     *
     **/
    int badMajor;

    /**
     *
     * The minor version number of the unsupported encoding.
     *
     **/
    int badMinor;

    /**
     *
     * The major version number of the encoding that is supported.
     *
     **/
    int major;

    /**
     *
     * The highest minor version number of the encoding that can be supported.
     *
     **/
    int minor;
};

/**
 *
 * This exception indicates that an unknown protocol message has been received.
 *
 **/
local exception UnknownMessageException extends ProtocolException
{
};

/**
 *
 * This exception is raised if a message is received over a connection
 * that is not yet validated.
 *
 **/
local exception ConnectionNotValidatedException extends ProtocolException
{
};

/**
 *
 * This exception indicates that a response for an unknown request ID has been
 * received.
 *
 **/
local exception UnknownRequestIdException extends ProtocolException
{
};

/**
 *
 * This exception indicates that an unknown reply status has been received.
 *
 **/
local exception UnknownReplyStatusException extends ProtocolException
{
};

/**
 *
 * This exception indicates that the connection has been gracefully shut down by the
 * server. The operation call that caused this exception has not been
 * executed by the server. In most cases you will not get this
 * exception, because the client will automatically retry the
 * operation call in case the server shut down the connection. However,
 * if upon retry the server shuts down the connection again, and the
 * retry limit has been reached, then this exception is propagated to
 * the application code.
 *
 **/
local exception CloseConnectionException extends ProtocolException
{
};

/**
 *
 * This exception is raised by an operation call if the application
 * forcefully closes the connection [Connection::close].
 *
 * @see Connection::close
 *
 **/
local exception ForcedCloseConnectionException extends ProtocolException
{
};

/**
 *
 * This exception indicates that a message size is less
 * than the minimum required size.
 *
 **/
local exception IllegalMessageSizeException extends ProtocolException
{
};

/**
 *
 * This exception indicates a problem with compressing or uncompressing data.
 *
 **/
local exception CompressionException extends ProtocolException
{
};

/**
 *
 * A datagram exceeds the configured size.
 *
 * This exception is raised if a datagram exceeds the configured send or receive buffer
 * size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
 *
 **/
local exception DatagramLimitException extends ProtocolException
{
};

/**
 *
 * This exception is raised for errors during marshaling or unmarshaling data.
 *
 **/
local exception MarshalException extends ProtocolException
{
};

/**
 *
 * This exception is raised if inconsistent data is received while unmarshaling a proxy.
 *
 **/
local exception ProxyUnmarshalException extends MarshalException
{
};

/**
 *
 * This exception is raised if an out-of-bounds condition occurs during unmarshaling.
 *
 **/
local exception UnmarshalOutOfBoundsException extends MarshalException
{
};

/**
 *
 * This exception indicates an illegal indirection during unmarshaling.
 *
 **/
local exception IllegalIndirectionException extends MarshalException
{
};

/**
 *
 * This exception is raised if no suitable object factory was found during
 * unmarshaling of a Slice class instance.
 *
 * @see ObjectFactory
 * @see Communicator::addObjectFactory
 * @see Communicator::findObjectFactory
 *
 **/
local exception NoObjectFactoryException extends MarshalException
{
    /**
     *
     * The Slice type ID of the class instance for which no
     * no factory could be found.
     *
     **/
    string type;
};

/**
 *
 * This exception is raised if the type of an unmarshaled Slice class instance does
 * not match its expected type.
 * This can happen if client and server are compiled with mismatched Slice
 * definitions or if a class of the wrong type is passed as a parameter
 * or return value using dynamic invocation. This exception can also be
 * raised if [IceStorm] is used to send Slice class instances and
 * an operation is subscribed to the wrong topic.
 *
 **/
local exception UnexpectedObjectException extends MarshalException
{
    /**
     *
     * The Slice type ID of the class instance that was unmarshaled.
     *
     **/
    string type;

    /**
     *
     * The Slice type ID that was excepted by the receiving operation.
     *
     **/
    string expectedType;
};

/**
 *
 * This exception is raised if the system-specific memory limit is exceeded during
 * marshaling or unmarshaling.
 *
 **/
local exception MemoryLimitException extends MarshalException
{
};

/**
 *
 * This exception is
 * raised when a string conversion to or from UTF-8 fails during 
 * marshaling or unmarshaling.
 *
 **/
local exception StringConversionException extends MarshalException
{
};


/**
 *
 * This exception indicates a malformed data encapsulation.
 *
 **/
local exception EncapsulationException extends MarshalException
{
};

/**
 *
 * This exception is raised if a negative size
 * (for example, a negative sequence size) is received.
 *
 **/
local exception NegativeSizeException extends MarshalException
{
};

/**
 *
 * This exception is raised if an unsupported feature is used. The
 * unsupported feature string contains the name of the unsupported
 * feature
 *
 **/
local exception FeatureNotSupportedException
{
    /**
     *
     * The name of the unsupported feature.
     *
     **/
    string unsupportedFeature;
};


/**
 *
 * This exception indicates a failure in a security subsystem,
 * such as the IceSSL plugin.
 *
 **/
local exception SecurityException
{
    /**
     *
     * The reason for the failure.
     *
     **/
    string reason;
};

/**
 * 
 * This exception indicates that an attempt has been made to
 * change the connection properties of a fixed proxy.
 *
 **/
local exception FixedProxyException
{
};

/**
 * 
 * Indicates that the response to a request has already been sent;
 * re-dispatching such a request is not possible.
 *
 **/
local exception ResponseSentException
{
};


};

#endif
