// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_LOCAL_EXCEPTION_ICE
#define ICE_LOCAL_EXCEPTION_ICE

#include <Ice/Identity.ice>
#include <Ice/Facet.ice>

module Ice
{

/**
 *
 * This exception is raised if an operation call on a server raises an
 * unknown exception. For example, for C++, this exception is raised
 * if the server throws a C++ exception that is not directly or
 * indirectly derived from <literal>Ice::LocalException</literal> or
 * <literal>Ice::UserException</literal>.
 *
 **/
local exception UnknownException
{
    /**
     *
     * The unknown exception as textual representation. This field may
     * or may not be set, depending on the security policy of the
     * server. Some servers may give this information to clients for
     * debugging purposes, while others don't provide this information
     * to prevent any information about the server internals.
     *
     **/
    string unknown;
};
    
/**
 *
 * This exception is raised if an operation call on a server raises a
 * local exception. Because local exceptions are not transmitted by
 * the &Ice; protocol, the client receives all local exceptions raised
 * by the server as [UknownLocalException]. The only exception to this
 * rule are all exceptions derived from [RequestFailedException],
 * which are transmitted by the &Ice; protocol even though they are
 * declared [local].
 *
 **/
local exception UnknownLocalException extends UnknownException
{
};

/**
 *
 * This exception is raised if an operation call on a server raises a
 * user exception which is not declared in the exception's
 * <literal>throws</literal> clause. Such undeclared exceptions are
 * not transmitted from the server to the client by the &Ice; protocol,
 * but instead the client just gets an [UnknownUserException]. This is
 * necessary in order to not violate the contract established by an
 * operation's signature: Only local exceptions and user exceptions
 * declared in the <literal>throws</literal> clause can be raised.
 *
 **/
local exception UnknownUserException extends UnknownException
{
};

/**
 *
 * This exception is raised if the &Ice; library version doesn't match
 * the &Ice; header files version.
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
};

/**
 *
 * This exception is raised only if the [ObjectAdapter] [Locator] is
 * set and if the [ObjectAdapter] activation failed because it's not
 * already registered with the [Locator].
 *
 **/
local exception ObjectAdapterNotRegisteredException
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
 * This exception is raised only if the [ObjectAdapter] [Locator] is
 * set and if the [ObjectAdapter] can't be activated because the
 * [Locator] detected another active [ObjectAdapter] with the same
 * name.
 *
 **/
local exception ObjectAdapterNameInUseException
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
 * This exception is raised if not suitable endpoint is available.
 *
 **/
local exception NoEndpointException
{
};

/**
 *
 * This exception is raised if there was an error while parsing an
 * endpoint.
 *
 **/
local exception EndpointParseException
{
};

/**
 *
 * This exception is raised if there was an error while parsing a
 * stringified proxy.
 *
 **/
local exception ProxyParseException
{
};

/**
 *
 * This exception is raised if an operation call using a proxy
 * resulted in a location forward to another proxy that doesn't
 * match this proxy's identity.
 *
 **/
local exception LocationForwardIdentityException
{
};

/**
 *
 * This exception is raised if a request failed. This exception, and
 * all exceptions derived from [RequestFailedException], is
 * transmitted by the &Ice; protocol, even though it is declared
 * [local].
 *
 **/
local exception RequestFailedException
{
    /** The identity of the Ice Object to which the request was sent to. */
    Identity id;

    /** The facet to which the request was sent to. */
    FacetPath facet;

    /** The operation name of the request. */
    string operation;
};

/**
 *
 * This exception is raised if an object does not exist on the server.
 *
 **/
local exception ObjectNotExistException extends RequestFailedException
{
};

/**
 *
 * This exception is raised if an object does not implement a given
 * facet path.
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
     * Unix, this is equivalent to <literal>errno</literal>. For C++
     * and Windows, this is the value returned by
     * <literal>GetLastError()</literal> or
     * <literal>WSAGetLastError()</literal>.
     *
     **/
    int error; // Don't use errno, as errno is usually a macro.
};

/**
 *
 * This exception is a specialization of [SyscallException] for socket
 * errors.
 *
 **/
local exception SocketException extends SyscallException
{
};

/**
 *
 * This exception is a specialization of [SocketException] for
 * connection failures.
 *
 **/
local exception ConnectFailedException extends SocketException
{
};

/**
 *
 * This exception is a specialization of [SocketException], indicating
 * a lost connection.
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
     * this is equivalent to <literal>h_errno</literal>. For C++ and
     * Windows, this is the value returned by
     * <literal>WSAGetLastError()</literal>.
     *
     **/
    int error; // Don't use h_errno, as h_errno is usually a macro.
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
 * This exception is a specialization of [TimeoutException] for
 * connection timeout conditions.
 *
 **/
local exception ConnectTimeoutException extends TimeoutException
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
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unsupported protocol version has been
 * encountered.
 *
 **/
local exception UnsupportedProtocolException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unsupported data encoding version has been
 * encountered.
 *
 **/
local exception UnsupportedEncodingException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unknown protocol message has been received.
 *
 **/
local exception UnknownMessageException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], that is
 * raised if a message is received over a connection that is not yet
 * validated.
 *
 **/
local exception ConnectionNotValidatedException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that a response for an unknown request id has been
 * received.
 *
 **/
local exception UnknownRequestIdException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that an unknown reply status been received.
 *
 **/
local exception UnknownReplyStatusException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that the connection has been gracefully shut down by the
 * server. The operation call that caused this exception has not been
 * executed by the server. In most cases you will not get this
 * exception, because the client will automatically retry the
 * operation call in case the the server shut down the
 * connection. However, if upon retry the server shuts down the
 * connection again, and the retry limit has been reached, then this
 * exception is propagated to the application code.
 *
 **/
local exception CloseConnectionException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that a batch request has been aborted.
 *
 **/
local exception AbortBatchRequestException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException],
 * indicating that the message size is illegal, i.e., it is
 * less than the minimum required size.
 *
 **/
local exception IllegalMessageSizeException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if a compressed protocol message has been received by an
 * &Ice; version which does not support compression.
 *
 **/
local exception CompressionNotSupportedException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised if there is a problem with compressing or uncompressing
 * data.
 *
 **/
local exception CompressionException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException], which is
 * raised upon an error during marshalling or unmarshaling data.
 *
 **/
local exception MarshalException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [MarshalException], which is
 * raised if no suitable object factory was found during object
 * unmarshaling.
 *
 * @see ObjectFactory
 * @see Communicator::addObjectFactory
 * @see Communicator::removeObjectFactory
 * @see Communicator::findObjectFactory
 *
 **/
local exception NoObjectFactoryException extends MarshalException
{
};

/**
 *
 * This exception is a specialization of [MarshalException], which is
 * raised if no suitable user exception factory was found during user
 * exception unmarshaling.
 *
 * @see UserExceptionFactory
 * @see Communicator::addUserExceptionFactory
 * @see Communicator::removeUserExceptionFactory
 * @see Communicator::findUserExceptionFactory
 *
 **/
local exception NoUserExceptionFactoryException extends MarshalException
{
};

/**
 *
 * This exception is a specialization of [MarshalException], which is
 * raised if inconsistent data is received while unmarshaling a proxy.
 *
 **/
local exception ProxyUnmarshalException extends MarshalException
{
};

/**
 *
 * This exception is a specialization of [MarshalException], which is
 * raised if an out-of-bounds condition occurs during unmarshaling.
 *
 **/
local exception UnmarshalOutOfBoundsException extends MarshalException
{
};

/**
 *
 * This exception is a specialization of [MarshalException],
 * indicating an illegal indirection during unmarshalling.
 *
 **/
local exception IllegalIndirectionException extends MarshalException
{
};

/**
 *
 * This exception is a specialization of [MarshalException], which is
 * raised if the system-specific memory limit is exceeded during
 * marshaling or unmarshaling.
 *
 **/
local exception MemoryLimitException extends MarshalException
{
};

/**
 *
 * This exception is a specialization of [MarshalException],
 * indicating a malformed data encapsulation.
 *
 **/
local exception EncapsulationException extends MarshalException
{
};

/**
 *
 * This exception is a specialization of [MarshalException], which is
 * raised if a negative size (e.g., a negative sequence size) is
 * received.
 *
 **/
local exception NegativeSizeException extends MarshalException
{
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
 * A plug-in already exists with the same name.
 *
 **/
local exception PluginExistsException
{
};

/**
 *
 * No plug-in exists with the given name.
 *
 **/
local exception PluginNotFoundException
{
};

};

#endif
