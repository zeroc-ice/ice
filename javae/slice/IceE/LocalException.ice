// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_LOCAL_EXCEPTION_ICE
#define ICEE_LOCAL_EXCEPTION_ICE

#include <IceE/Identity.ice>
#include <IceE/BuiltinSequences.ice>

module Ice
{

/**
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
     * The id (or name) of the object that is registered already.
     *
     **/
    string id;
};

/**
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
     * The id (or name) of the object that could not be removed.
     *
     **/
    string id;
};

/**
 *
 * This exception is raised if an attempt is made to invoke an
 * operation with [ice_oneway], [ice_batchOneway], [ice_datagram],
 * or [ice_batchDatagram] and the operation has a return value,
 * out parameters, or an exception specification.
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
 * indirectly derived from <literal>Ice::LocalException</literal> or
 * <literal>Ice::UserException</literal>.
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
 * the &Ice; protocol, the client receives all local exceptions raised
 * by the server as [UnknownLocalException]. The only exception to this
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
 * user exception that is not declared in the exception's
 * <literal>throws</literal> clause. Such undeclared exceptions are
 * not transmitted from the server to the client by the &Ice;
 * protocol, but instead the client just gets an
 * [UnknownUserException]. This is necessary in order to not violate
 * the contract established by an operation's signature: Only local
 * exceptions and user exceptions declared in the
 * <literal>throws</literal> clause can be raised.
 *
 **/
local exception UnknownUserException extends UnknownException
{
};

/**
 *
 * This exception is raised if the &Ice; library version does not match
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
    /**
     * 
     * Name of the adapter.
     *
     **/
    string name;
};

/**
 *
 * This exception is raised if an [ObjectAdapter] cannot be activated
 * because the [Locator] detected another active [ObjectAdapter] with
 * the same adapter id.
 *
 **/
local exception ObjectAdapterIdInUseException
{
    /**
     *
     * Adapter id.
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
 * transmitted by the &Ice; protocol, even though they are declared
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
 * This exception is a specialization of [SyscallException] for file
 * errors.
 *
 **/
local exception FileException extends SyscallException
{
    /** The path of the file responsible for the error. */
    string path;
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
 * This exception is a specialization of [ConnectFailedException] for
 * connection failures, where the server host actively refuses a
 * connection.
 *
 **/
local exception ConnectionRefusedException extends ConnectFailedException
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
 * This exception is a specialization of [TimeoutException] for
 * connection establishment timeout conditions.
 *
 **/
local exception ConnectTimeoutException extends TimeoutException
{
};

/**
 *
 * This exception is a specialization of [TimeoutException] for
 * connection closure timeout conditions.
 *
 **/
local exception CloseTimeoutException extends TimeoutException
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
 * This exception is a specialization of [ProtocolException],
 * indicating that the connection has been gracefully shut down by the
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
 * forcefully closes the connection used by this call with
 * [Connection::close].
 *
 * @see Connection::close
 *
 **/
local exception ForcedCloseConnectionException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [ProtocolException] that is
 * raised upon an error during marshaling or unmarshaling data.
 *
 **/
local exception MarshalException extends ProtocolException
{
};

/**
 *
 * This exception is a specialization of [MarshalException] that is
 * raised if the system-specific memory limit is exceeded during
 * marshaling or unmarshaling.
 *
 **/
local exception MemoryLimitException extends MarshalException
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
 * This exception indicates that an attempt has been made to
 * change the connection properties of a fixed proxy.
 *
 **/
local exception FixedProxyException
{
};

};

#endif
