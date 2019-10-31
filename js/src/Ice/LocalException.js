//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
const Ice = _ModuleRegistry.require(module,
                                    [
                                        "../Ice/Exception",
                                        "../Ice/Long",
                                        "../Ice/HashMap",
                                        "../Ice/HashUtil",
                                        "../Ice/ArrayUtil",
                                        "../Ice/StreamHelpers",
                                        "../Ice/Identity",
                                        "../Ice/Version",
                                        "../Ice/BuiltinSequences"
                                    ]).Ice;

const Slice = Ice.Slice;

Ice.InitializationException = class extends Ice.LocalException
{
    constructor(reason = "", _cause = "")
    {
        super(_cause);
        this.reason = reason;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::InitializationException";
    }
};

/**
 * This exception indicates that a failure occurred while initializing
 * a plug-in.
 *
 **/
Ice.PluginInitializationException = class extends Ice.LocalException
{
    constructor(reason = "", _cause = "")
    {
        super(_cause);
        this.reason = reason;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::PluginInitializationException";
    }
};

/**
 * This exception is raised if a feature is requested that is not
 * supported with collocation optimization.
 *
 * @deprecated This exception is no longer used by the Ice run time
 **/
Ice.CollocationOptimizationException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::CollocationOptimizationException";
    }
};

/**
 * An attempt was made to register something more than once with
 * the Ice run time.
 *
 * This exception is raised if an attempt is made to register a
 * servant, servant locator, facet, value factory, plug-in, object
 * adapter, object, or user exception factory more than once for the
 * same ID.
 *
 **/
Ice.AlreadyRegisteredException = class extends Ice.LocalException
{
    constructor(kindOfObject = "", id = "", _cause = "")
    {
        super(_cause);
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::AlreadyRegisteredException";
    }
};

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
 *
 **/
Ice.NotRegisteredException = class extends Ice.LocalException
{
    constructor(kindOfObject = "", id = "", _cause = "")
    {
        super(_cause);
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::NotRegisteredException";
    }
};

/**
 * The operation can only be invoked with a twoway request.
 *
 * This exception is raised if an attempt is made to invoke an
 * operation with <code>ice_oneway</code>, <code>ice_batchOneway</code>, <code>ice_datagram</code>,
 * or <code>ice_batchDatagram</code> and the operation has a return value,
 * out-parameters, or an exception specification.
 *
 **/
Ice.TwowayOnlyException = class extends Ice.LocalException
{
    constructor(operation = "", _cause = "")
    {
        super(_cause);
        this.operation = operation;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::TwowayOnlyException";
    }
};

/**
 * An attempt was made to clone a class that does not support
 * cloning.
 *
 * This exception is raised if <code>ice_clone</code> is called on
 * a class that is derived from an abstract Slice class (that is,
 * a class containing operations), and the derived class does not
 * provide an implementation of the <code>ice_clone</code> operation (C++ only).
 *
 **/
Ice.CloneNotImplementedException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::CloneNotImplementedException";
    }
};

/**
 * This exception is raised if an operation call on a server raises an
 * unknown exception. For example, for C++, this exception is raised
 * if the server throws a C++ exception that is not directly or
 * indirectly derived from <code>Ice::LocalException</code> or
 * <code>Ice::UserException</code>.
 *
 **/
Ice.UnknownException = class extends Ice.LocalException
{
    constructor(unknown = "", _cause = "")
    {
        super(_cause);
        this.unknown = unknown;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::UnknownException";
    }
};

/**
 * This exception is raised if an operation call on a server raises a
 * local exception. Because local exceptions are not transmitted by
 * the Ice protocol, the client receives all local exceptions raised
 * by the server as {@link UnknownLocalException}. The only exception to this
 * rule are all exceptions derived from {@link RequestFailedException},
 * which are transmitted by the Ice protocol even though they are
 * declared <code>local</code>.
 *
 **/
Ice.UnknownLocalException = class extends Ice.UnknownException
{
    constructor(unknown, _cause = "")
    {
        super(unknown, _cause);
    }

    static get _parent()
    {
        return Ice.UnknownException;
    }

    static get _id()
    {
        return "::Ice::UnknownLocalException";
    }
};

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
 *
 **/
Ice.UnknownUserException = class extends Ice.UnknownException
{
    constructor(unknown, _cause = "")
    {
        super(unknown, _cause);
    }

    static get _parent()
    {
        return Ice.UnknownException;
    }

    static get _id()
    {
        return "::Ice::UnknownUserException";
    }
};

/**
 * This exception is raised if the Ice library version does not match
 * the version in the Ice header files.
 *
 **/
Ice.VersionMismatchException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::VersionMismatchException";
    }
};

/**
 * This exception is raised if the {@link Communicator} has been destroyed.
 *
 * @see Communicator#destroy
 *
 **/
Ice.CommunicatorDestroyedException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::CommunicatorDestroyedException";
    }
};

/**
 * This exception is raised if an attempt is made to use a deactivated
 * {@link ObjectAdapter}.
 *
 * @see ObjectAdapter#deactivate
 * @see Communicator#shutdown
 *
 **/
Ice.ObjectAdapterDeactivatedException = class extends Ice.LocalException
{
    constructor(name = "", _cause = "")
    {
        super(_cause);
        this.name = name;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::ObjectAdapterDeactivatedException";
    }
};

/**
 * This exception is raised if an {@link ObjectAdapter} cannot be activated.
 *
 * This happens if the {@link Locator} detects another active {@link ObjectAdapter} with
 * the same adapter id.
 *
 **/
Ice.ObjectAdapterIdInUseException = class extends Ice.LocalException
{
    constructor(id = "", _cause = "")
    {
        super(_cause);
        this.id = id;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::ObjectAdapterIdInUseException";
    }
};

/**
 * This exception is raised if no suitable endpoint is available.
 *
 **/
Ice.NoEndpointException = class extends Ice.LocalException
{
    constructor(proxy = "", _cause = "")
    {
        super(_cause);
        this.proxy = proxy;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::NoEndpointException";
    }
};

/**
 * This exception is raised if there was an error while parsing an
 * endpoint.
 *
 **/
Ice.EndpointParseException = class extends Ice.LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::EndpointParseException";
    }
};

/**
 * This exception is raised if there was an error while parsing an
 * endpoint selection type.
 *
 **/
Ice.EndpointSelectionTypeParseException = class extends Ice.LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::EndpointSelectionTypeParseException";
    }
};

/**
 * This exception is raised if there was an error while parsing a
 * version.
 *
 **/
Ice.VersionParseException = class extends Ice.LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::VersionParseException";
    }
};

/**
 * This exception is raised if there was an error while parsing a
 * stringified identity.
 *
 **/
Ice.IdentityParseException = class extends Ice.LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::IdentityParseException";
    }
};

/**
 * This exception is raised if there was an error while parsing a
 * stringified proxy.
 *
 **/
Ice.ProxyParseException = class extends Ice.LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::ProxyParseException";
    }
};

/**
 * This exception is raised if an illegal identity is encountered.
 *
 **/
Ice.IllegalIdentityException = class extends Ice.LocalException
{
    constructor(id = new Ice.Identity(), _cause = "")
    {
        super(_cause);
        this.id = id;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::IllegalIdentityException";
    }
};

/**
 * This exception is raised to reject an illegal servant (typically
 * a null servant)
 *
 **/
Ice.IllegalServantException = class extends Ice.LocalException
{
    constructor(reason = "", _cause = "")
    {
        super(_cause);
        this.reason = reason;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::IllegalServantException";
    }
};

/**
 * This exception is raised if a request failed. This exception, and
 * all exceptions derived from {@link RequestFailedException}, are
 * transmitted by the Ice protocol, even though they are declared
 * <code>local</code>.
 *
 **/
Ice.RequestFailedException = class extends Ice.LocalException
{
    constructor(id = new Ice.Identity(), facet = "", operation = "", _cause = "")
    {
        super(_cause);
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::RequestFailedException";
    }
};

/**
 * This exception is raised if an object does not exist on the server,
 * that is, if no facets with the given identity exist.
 *
 **/
Ice.ObjectNotExistException = class extends Ice.RequestFailedException
{
    constructor(id, facet, operation, _cause = "")
    {
        super(id, facet, operation, _cause);
    }

    static get _parent()
    {
        return Ice.RequestFailedException;
    }

    static get _id()
    {
        return "::Ice::ObjectNotExistException";
    }
};

/**
 * This exception is raised if no facet with the given name exists,
 * but at least one facet with the given identity exists.
 *
 **/
Ice.FacetNotExistException = class extends Ice.RequestFailedException
{
    constructor(id, facet, operation, _cause = "")
    {
        super(id, facet, operation, _cause);
    }

    static get _parent()
    {
        return Ice.RequestFailedException;
    }

    static get _id()
    {
        return "::Ice::FacetNotExistException";
    }
};

/**
 * This exception is raised if an operation for a given object does
 * not exist on the server. Typically this is caused by either the
 * client or the server using an outdated Slice specification.
 *
 **/
Ice.OperationNotExistException = class extends Ice.RequestFailedException
{
    constructor(id, facet, operation, _cause = "")
    {
        super(id, facet, operation, _cause);
    }

    static get _parent()
    {
        return Ice.RequestFailedException;
    }

    static get _id()
    {
        return "::Ice::OperationNotExistException";
    }
};

/**
 * This exception is raised if a system error occurred in the server
 * or client process. There are many possible causes for such a system
 * exception. For details on the cause, {@link SyscallException#error}
 * should be inspected.
 *
 **/
Ice.SyscallException = class extends Ice.LocalException
{
    constructor(error = 0, _cause = "")
    {
        super(_cause);
        this.error = error;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::SyscallException";
    }
};

/**
 * This exception indicates socket errors.
 *
 **/
Ice.SocketException = class extends Ice.SyscallException
{
    constructor(error, _cause = "")
    {
        super(error, _cause);
    }

    static get _parent()
    {
        return Ice.SyscallException;
    }

    static get _id()
    {
        return "::Ice::SocketException";
    }
};

/**
 * This exception indicates CFNetwork errors.
 *
 **/
Ice.CFNetworkException = class extends Ice.SocketException
{
    constructor(error, domain = "", _cause = "")
    {
        super(error, _cause);
        this.domain = domain;
    }

    static get _parent()
    {
        return Ice.SocketException;
    }

    static get _id()
    {
        return "::Ice::CFNetworkException";
    }
};

/**
 * This exception indicates file errors.
 *
 **/
Ice.FileException = class extends Ice.SyscallException
{
    constructor(error, path = "", _cause = "")
    {
        super(error, _cause);
        this.path = path;
    }

    static get _parent()
    {
        return Ice.SyscallException;
    }

    static get _id()
    {
        return "::Ice::FileException";
    }
};

/**
 * This exception indicates connection failures.
 *
 **/
Ice.ConnectFailedException = class extends Ice.SocketException
{
    constructor(error, _cause = "")
    {
        super(error, _cause);
    }

    static get _parent()
    {
        return Ice.SocketException;
    }

    static get _id()
    {
        return "::Ice::ConnectFailedException";
    }
};

/**
 * This exception indicates a connection failure for which
 * the server host actively refuses a connection.
 *
 **/
Ice.ConnectionRefusedException = class extends Ice.ConnectFailedException
{
    constructor(error, _cause = "")
    {
        super(error, _cause);
    }

    static get _parent()
    {
        return Ice.ConnectFailedException;
    }

    static get _id()
    {
        return "::Ice::ConnectionRefusedException";
    }
};

/**
 * This exception indicates a lost connection.
 *
 **/
Ice.ConnectionLostException = class extends Ice.SocketException
{
    constructor(error, _cause = "")
    {
        super(error, _cause);
    }

    static get _parent()
    {
        return Ice.SocketException;
    }

    static get _id()
    {
        return "::Ice::ConnectionLostException";
    }
};

/**
 * This exception indicates a DNS problem. For details on the cause,
 * {@link DNSException#error} should be inspected.
 *
 **/
Ice.DNSException = class extends Ice.LocalException
{
    constructor(error = 0, host = "", _cause = "")
    {
        super(_cause);
        this.error = error;
        this.host = host;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::DNSException";
    }
};

/**
 * This exception indicates a request was interrupted.
 *
 **/
Ice.OperationInterruptedException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::OperationInterruptedException";
    }
};

/**
 * This exception indicates a timeout condition.
 *
 **/
Ice.TimeoutException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::TimeoutException";
    }
};

/**
 * This exception indicates a connection establishment timeout condition.
 *
 **/
Ice.ConnectTimeoutException = class extends Ice.TimeoutException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.TimeoutException;
    }

    static get _id()
    {
        return "::Ice::ConnectTimeoutException";
    }
};

/**
 * This exception indicates a connection closure timeout condition.
 *
 **/
Ice.CloseTimeoutException = class extends Ice.TimeoutException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.TimeoutException;
    }

    static get _id()
    {
        return "::Ice::CloseTimeoutException";
    }
};

/**
 * This exception indicates that a connection has been shut down because it has been
 * idle for some time.
 *
 **/
Ice.ConnectionTimeoutException = class extends Ice.TimeoutException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.TimeoutException;
    }

    static get _id()
    {
        return "::Ice::ConnectionTimeoutException";
    }
};

/**
 * This exception indicates that an invocation failed because it timed
 * out.
 *
 **/
Ice.InvocationTimeoutException = class extends Ice.TimeoutException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.TimeoutException;
    }

    static get _id()
    {
        return "::Ice::InvocationTimeoutException";
    }
};

/**
 * This exception indicates that an asynchronous invocation failed
 * because it was canceled explicitly by the user.
 *
 **/
Ice.InvocationCanceledException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::InvocationCanceledException";
    }
};

/**
 * A generic exception base for all kinds of protocol error
 * conditions.
 *
 **/
Ice.ProtocolException = class extends Ice.LocalException
{
    constructor(reason = "", _cause = "")
    {
        super(_cause);
        this.reason = reason;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::ProtocolException";
    }
};

/**
 * This exception indicates that a message did not start with the expected
 * magic number ('I', 'c', 'e', 'P').
 *
 **/
Ice.BadMagicException = class extends Ice.ProtocolException
{
    constructor(reason, badMagic = null, _cause = "")
    {
        super(reason, _cause);
        this.badMagic = badMagic;
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::BadMagicException";
    }
};

/**
 * This exception indicates an unsupported protocol version.
 *
 **/
Ice.UnsupportedProtocolException = class extends Ice.ProtocolException
{
    constructor(reason, bad = new Ice.ProtocolVersion(), supported = new Ice.ProtocolVersion(), _cause = "")
    {
        super(reason, _cause);
        this.bad = bad;
        this.supported = supported;
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::UnsupportedProtocolException";
    }
};

/**
 * This exception indicates an unsupported data encoding version.
 *
 **/
Ice.UnsupportedEncodingException = class extends Ice.ProtocolException
{
    constructor(reason, bad = new Ice.EncodingVersion(), supported = new Ice.EncodingVersion(), _cause = "")
    {
        super(reason, _cause);
        this.bad = bad;
        this.supported = supported;
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::UnsupportedEncodingException";
    }
};

/**
 * This exception indicates that an unknown protocol message has been received.
 *
 **/
Ice.UnknownMessageException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::UnknownMessageException";
    }
};

/**
 * This exception is raised if a message is received over a connection
 * that is not yet validated.
 *
 **/
Ice.ConnectionNotValidatedException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::ConnectionNotValidatedException";
    }
};

/**
 * This exception indicates that a response for an unknown request ID has been
 * received.
 *
 **/
Ice.UnknownRequestIdException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::UnknownRequestIdException";
    }
};

/**
 * This exception indicates that an unknown reply status has been received.
 *
 **/
Ice.UnknownReplyStatusException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::UnknownReplyStatusException";
    }
};

/**
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
Ice.CloseConnectionException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::CloseConnectionException";
    }
};

/**
 * This exception is raised by an operation call if the application
 * closes the connection locally using {@link Connection#close}.
 *
 * @see Connection#close
 *
 **/
Ice.ConnectionManuallyClosedException = class extends Ice.LocalException
{
    constructor(graceful = false, _cause = "")
    {
        super(_cause);
        this.graceful = graceful;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::ConnectionManuallyClosedException";
    }
};

/**
 * This exception indicates that a message size is less
 * than the minimum required size.
 *
 **/
Ice.IllegalMessageSizeException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::IllegalMessageSizeException";
    }
};

/**
 * This exception indicates a problem with compressing or uncompressing data.
 *
 **/
Ice.CompressionException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::CompressionException";
    }
};

/**
 * A datagram exceeds the configured size.
 *
 * This exception is raised if a datagram exceeds the configured send or receive buffer
 * size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
 *
 **/
Ice.DatagramLimitException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::DatagramLimitException";
    }
};

/**
 * This exception is raised for errors during marshaling or unmarshaling data.
 *
 **/
Ice.MarshalException = class extends Ice.ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.ProtocolException;
    }

    static get _id()
    {
        return "::Ice::MarshalException";
    }
};

/**
 * This exception is raised if inconsistent data is received while unmarshaling a proxy.
 *
 **/
Ice.ProxyUnmarshalException = class extends Ice.MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.MarshalException;
    }

    static get _id()
    {
        return "::Ice::ProxyUnmarshalException";
    }
};

/**
 * This exception is raised if an out-of-bounds condition occurs during unmarshaling.
 *
 **/
Ice.UnmarshalOutOfBoundsException = class extends Ice.MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.MarshalException;
    }

    static get _id()
    {
        return "::Ice::UnmarshalOutOfBoundsException";
    }
};

/**
 * This exception is raised if no suitable value factory was found during
 * unmarshaling of a Slice class instance.
 *
 * @see ValueFactory
 * @see Communicator#getValueFactoryManager
 * @see ValueFactoryManager#add
 * @see ValueFactoryManager#find
 *
 **/
Ice.NoValueFactoryException = class extends Ice.MarshalException
{
    constructor(reason, type = "", _cause = "")
    {
        super(reason, _cause);
        this.type = type;
    }

    static get _parent()
    {
        return Ice.MarshalException;
    }

    static get _id()
    {
        return "::Ice::NoValueFactoryException";
    }
};

/**
 * This exception is raised if the type of an unmarshaled Slice class instance does
 * not match its expected type.
 * This can happen if client and server are compiled with mismatched Slice
 * definitions or if a class of the wrong type is passed as a parameter
 * or return value using dynamic invocation. This exception can also be
 * raised if IceStorm is used to send Slice class instances and
 * an operation is subscribed to the wrong topic.
 *
 **/
Ice.UnexpectedObjectException = class extends Ice.MarshalException
{
    constructor(reason, type = "", expectedType = "", _cause = "")
    {
        super(reason, _cause);
        this.type = type;
        this.expectedType = expectedType;
    }

    static get _parent()
    {
        return Ice.MarshalException;
    }

    static get _id()
    {
        return "::Ice::UnexpectedObjectException";
    }
};

/**
 * This exception is raised when Ice receives a request or reply
 * message whose size exceeds the limit specified by the
 * <code>Ice.MessageSizeMax</code> property.
 *
 **/
Ice.MemoryLimitException = class extends Ice.MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.MarshalException;
    }

    static get _id()
    {
        return "::Ice::MemoryLimitException";
    }
};

/**
 * This exception is raised when a string conversion to or from UTF-8
 * fails during marshaling or unmarshaling.
 *
 **/
Ice.StringConversionException = class extends Ice.MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.MarshalException;
    }

    static get _id()
    {
        return "::Ice::StringConversionException";
    }
};

/**
 * This exception indicates a malformed data encapsulation.
 *
 **/
Ice.EncapsulationException = class extends Ice.MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _parent()
    {
        return Ice.MarshalException;
    }

    static get _id()
    {
        return "::Ice::EncapsulationException";
    }
};

/**
 * This exception is raised if an unsupported feature is used. The
 * unsupported feature string contains the name of the unsupported
 * feature
 *
 **/
Ice.FeatureNotSupportedException = class extends Ice.LocalException
{
    constructor(unsupportedFeature = "", _cause = "")
    {
        super(_cause);
        this.unsupportedFeature = unsupportedFeature;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::FeatureNotSupportedException";
    }
};

/**
 * This exception indicates a failure in a security subsystem,
 * such as the IceSSL plug-in.
 *
 **/
Ice.SecurityException = class extends Ice.LocalException
{
    constructor(reason = "", _cause = "")
    {
        super(_cause);
        this.reason = reason;
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::SecurityException";
    }
};

/**
 * This exception indicates that an attempt has been made to
 * change the connection properties of a fixed proxy.
 *
 **/
Ice.FixedProxyException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::FixedProxyException";
    }
};

/**
 * Indicates that the response to a request has already been sent;
 * re-dispatching such a request is not possible.
 *
 **/
Ice.ResponseSentException = class extends Ice.LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _parent()
    {
        return Ice.LocalException;
    }

    static get _id()
    {
        return "::Ice::ResponseSentException";
    }
};

exports.Ice = Ice;
