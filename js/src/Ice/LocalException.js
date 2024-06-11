//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { LocalException } from "./Exception.js";
import { Ice as Ice_Identity } from "./Identity.js";
import { Ice as Ice_Version } from "./Version.js";

const Ice = {
    ...Ice_Identity,
    ...Ice_Version,
};

/**
 *  This exception is raised when a failure occurs during initialization.
 **/
export class InitializationException extends LocalException
{
    constructor(reason = "", _cause = "")
    {
        super(_cause);
        this.reason = reason;
    }

    static get _id()
    {
        return "::Ice::InitializationException";
    }
}

/**
 *  An attempt was made to register something more than once with the Ice run time. This exception is raised if an
 *  attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
 *  user exception factory more than once for the same ID.
 **/
export class AlreadyRegisteredException extends LocalException
{
    constructor(kindOfObject = "", id = "", _cause = "")
    {
        super(_cause);
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    static get _id()
    {
        return "::Ice::AlreadyRegisteredException";
    }
}

/**
 *  An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
 *  This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
 *  object adapter, object, or user exception factory that is not currently registered. It's also raised if the Ice
 *  locator can't find an object or object adapter when resolving an indirect proxy or when an object adapter is
 *  activated.
 **/
export class NotRegisteredException extends LocalException
{
    constructor(kindOfObject = "", id = "", _cause = "")
    {
        super(_cause);
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    static get _id()
    {
        return "::Ice::NotRegisteredException";
    }
}

/**
 *  The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
 *  an operation with <code>ice_oneway</code>, <code>ice_batchOneway</code>, <code>ice_datagram</code>, or
 *  <code>ice_batchDatagram</code> and the operation has a return value, out-parameters, or an exception specification.
 **/
export class TwowayOnlyException extends LocalException
{
    constructor(operation = "", _cause = "")
    {
        super(_cause);
        this.operation = operation;
    }

    static get _id()
    {
        return "::Ice::TwowayOnlyException";
    }
}

/**
 *  This exception is raised if an operation call on a server raises an unknown exception. For example, for C++, this
 *  exception is raised if the server throws a C++ exception that is not directly or indirectly derived from
 *  <code>Ice::LocalException</code> or <code>Ice::UserException</code>.
 **/
export class UnknownException extends LocalException
{
    constructor(unknown = "", _cause = "")
    {
        super(_cause);
        this.unknown = unknown;
    }

    static get _id()
    {
        return "::Ice::UnknownException";
    }
}

/**
 *  This exception is raised if an operation call on a server raises a  local exception. Because local exceptions are
 *  not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
 *  {@link UnknownLocalException}. The only exception to this rule are all exceptions derived from
 *  {@link RequestFailedException}, which are transmitted by the Ice protocol even though they are declared
 *  <code>local</code>.
 **/
export class UnknownLocalException extends UnknownException
{
    constructor(unknown, _cause = "")
    {
        super(unknown, _cause);
    }

    static get _id()
    {
        return "::Ice::UnknownLocalException";
    }
}

/**
 *  An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
 *  that is not declared in the exception's <code>throws</code> clause. Such undeclared exceptions are not transmitted
 *  from the server to the client by the Ice protocol, but instead the client just gets an {@link UnknownUserException}.
 *  This is necessary in order to not violate the contract established by an operation's signature: Only local
 *  exceptions and user exceptions declared in the <code>throws</code> clause can be raised.
 **/
export class UnknownUserException extends UnknownException
{
    constructor(unknown, _cause = "")
    {
        super(unknown, _cause);
    }

    static get _id()
    {
        return "::Ice::UnknownUserException";
    }
}

/**
 *  This exception is raised if the {@link Communicator} has been destroyed.
 *  @see Communicator#destroy
 **/
export class CommunicatorDestroyedException extends LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::CommunicatorDestroyedException";
    }
}

/**
 *  This exception is raised if an attempt is made to use a deactivated {@link ObjectAdapter}.
 *  @see ObjectAdapter#deactivate
 *  @see Communicator#shutdown
 **/
export class ObjectAdapterDeactivatedException extends LocalException
{
    constructor(name = "", _cause = "")
    {
        super(_cause);
        this.name = name;
    }

    static get _id()
    {
        return "::Ice::ObjectAdapterDeactivatedException";
    }
}

/**
 *  This exception is raised if an {@link ObjectAdapter} cannot be activated. This happens if the {@link Locator}
 *  detects another active {@link ObjectAdapter} with the same adapter id.
 **/
export class ObjectAdapterIdInUseException extends LocalException
{
    constructor(id = "", _cause = "")
    {
        super(_cause);
        this.id = id;
    }

    static get _id()
    {
        return "::Ice::ObjectAdapterIdInUseException";
    }
}

/**
 *  This exception is raised if no suitable endpoint is available.
 **/
export class NoEndpointException extends LocalException
{
    constructor(proxy = "", _cause = "")
    {
        super(_cause);
        this.proxy = proxy;
    }

    static get _id()
    {
        return "::Ice::NoEndpointException";
    }
}

/**
 *  This exception is raised if there was an error while parsing an endpoint.
 **/
export class EndpointParseException extends LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _id()
    {
        return "::Ice::EndpointParseException";
    }
}

/**
 *  This exception is raised if there was an error while parsing an endpoint selection type.
 **/
export class EndpointSelectionTypeParseException extends LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _id()
    {
        return "::Ice::EndpointSelectionTypeParseException";
    }
}

/**
 *  This exception is raised if there was an error while parsing a version.
 **/
export class VersionParseException extends LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _id()
    {
        return "::Ice::VersionParseException";
    }
};

/**
 *  This exception is raised if there was an error while parsing a stringified identity.
 **/
export class IdentityParseException extends LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _id()
    {
        return "::Ice::IdentityParseException";
    }
}

/**
 *  This exception is raised if there was an error while parsing a stringified proxy.
 **/
export class ProxyParseException extends LocalException
{
    constructor(str = "", _cause = "")
    {
        super(_cause);
        this.str = str;
    }

    static get _id()
    {
        return "::Ice::ProxyParseException";
    }
}

/**
 *   This exception is raised if an illegal identity is encountered.
 **/
export class IllegalIdentityException extends LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::IllegalIdentityException";
    }
}

/**
 *  This exception is raised to reject an illegal servant (typically a null servant).
 **/
export class IllegalServantException extends LocalException
{
    constructor(reason = "", _cause = "")
    {
        super(_cause);
        this.reason = reason;
    }

    static get _id()
    {
        return "::Ice::IllegalServantException";
    }
}

/**
 *  This exception is raised if a request failed. This exception, and all exceptions derived from
 *  {@link RequestFailedException}, are transmitted by the Ice protocol, even though they are declared
 *  <code>local</code>.
 **/
export class RequestFailedException extends LocalException
{
    constructor(id = new Ice.Identity(), facet = "", operation = "", _cause = "")
    {
        super(_cause);
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    static get _id()
    {
        return "::Ice::RequestFailedException";
    }
}

/**
 *  This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
 *  exist.
 **/
export class ObjectNotExistException extends RequestFailedException
{
    constructor(id, facet, operation, _cause = "")
    {
        super(id, facet, operation, _cause);
    }

    static get _id()
    {
        return "::Ice::ObjectNotExistException";
    }
}

/**
 *  This exception is raised if no facet with the given name exists, but at least one facet with the given identity
 *  exists.
 **/
export class FacetNotExistException extends RequestFailedException
{
    constructor(id, facet, operation, _cause = "")
    {
        super(id, facet, operation, _cause);
    }

    static get _id()
    {
        return "::Ice::FacetNotExistException";
    }
}

/**
 *  This exception is raised if an operation for a given object does not exist on the server. Typically this is caused
 *  by either the client or the server using an outdated Slice specification.
 **/
export class OperationNotExistException extends RequestFailedException
{
    constructor(id, facet, operation, _cause = "")
    {
        super(id, facet, operation, _cause);
    }

    static get _id()
    {
        return "::Ice::OperationNotExistException";
    }
}

/**
 *  This exception is raised if a system error occurred in the server or client process. There are many possible causes
 *  for such a system exception. For details on the cause, {@link SyscallException#error} should be inspected.
 **/
export class SyscallException extends LocalException
{
    constructor(error = 0, _cause = "")
    {
        super(_cause);
        this.error = error;
    }

    static get _id()
    {
        return "::Ice::SyscallException";
    }
}

/**
 *  This exception indicates socket errors.
 **/
export class SocketException extends SyscallException
{
    constructor(error, _cause = "")
    {
        super(error, _cause);
    }

    static get _id()
    {
        return "::Ice::SocketException";
    }
}

/**
 *  This exception indicates connection failures.
 **/
export class ConnectFailedException extends SocketException
{
    constructor(error, _cause = "")
    {
        super(error, _cause);
    }

    static get _id()
    {
        return "::Ice::ConnectFailedException";
    }
}

/**
 *  This exception indicates a connection failure for which the server host actively refuses a connection.
 **/
export class ConnectionRefusedException extends ConnectFailedException
{
    constructor(error, _cause = "")
    {
        super(error, _cause);
    }

    static get _id()
    {
        return "::Ice::ConnectionRefusedException";
    }
}

/**
 *  This exception indicates a lost connection.
 **/
export class ConnectionLostException extends SocketException
{
    constructor(error, _cause = "")
    {
        super(error, _cause);
    }

    static get _id()
    {
        return "::Ice::ConnectionLostException";
    }
}

/**
 *  This exception indicates a timeout condition.
 **/
export class TimeoutException extends LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::TimeoutException";
    }
}

/**
 *  This exception indicates a connection establishment timeout condition.
 **/
export class ConnectTimeoutException extends TimeoutException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::ConnectTimeoutException";
    }
}

/**
 *  This exception indicates a connection closure timeout condition.
 **/
export class CloseTimeoutException extends TimeoutException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::CloseTimeoutException";
    }
};

/**
 *  This exception indicates that a connection has been shut down because it has been idle for some time.
 **/
export class ConnectionTimeoutException extends TimeoutException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::ConnectionTimeoutException";
    }
}

/**
 *  This exception indicates that an invocation failed because it timed out.
 **/
export class InvocationTimeoutException extends TimeoutException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::InvocationTimeoutException";
    }
}

/**
 *  This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
 **/
export class InvocationCanceledException extends LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::InvocationCanceledException";
    }
};

/**
 *  A generic exception base for all kinds of protocol error conditions.
 **/
export class ProtocolException extends LocalException
{
    constructor(reason = "", _cause = "")
    {
        super(_cause);
        this.reason = reason;
    }

    static get _id()
    {
        return "::Ice::ProtocolException";
    }
}

/**
 *  This exception indicates that a message did not start with the expected magic number ('I', 'c', 'e', 'P').
 **/
export class BadMagicException extends ProtocolException
{
    constructor(reason, badMagic = null, _cause = "")
    {
        super(reason, _cause);
        this.badMagic = badMagic;
    }

    static get _id()
    {
        return "::Ice::BadMagicException";
    }
}

/**
 *  This exception indicates an unsupported protocol version.
 **/
export class UnsupportedProtocolException extends ProtocolException
{
    constructor(reason, bad = new Ice.ProtocolVersion(), supported = new Ice.ProtocolVersion(), _cause = "")
    {
        super(reason, _cause);
        this.bad = bad;
        this.supported = supported;
    }

    static get _id()
    {
        return "::Ice::UnsupportedProtocolException";
    }
}

/**
 *  This exception indicates an unsupported data encoding version.
 **/
export class UnsupportedEncodingException extends ProtocolException
{
    constructor(reason, bad = new Ice.EncodingVersion(), supported = new Ice.EncodingVersion(), _cause = "")
    {
        super(reason, _cause);
        this.bad = bad;
        this.supported = supported;
    }

    static get _id()
    {
        return "::Ice::UnsupportedEncodingException";
    }
}

/**
 *  This exception indicates that an unknown protocol message has been received.
 **/
export class UnknownMessageException extends ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::UnknownMessageException";
    }
}

/**
 *  This exception is raised if a message is received over a connection that is not yet validated.
 **/
export class ConnectionNotValidatedException extends ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::ConnectionNotValidatedException";
    }
}

/**
 *  This exception indicates that an unknown reply status has been received.
 **/
export class UnknownReplyStatusException extends ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::UnknownReplyStatusException";
    }
}

/**
 *  This exception indicates that the connection has been gracefully shut down by the server. The operation call that
 *  caused this exception has not been executed by the server. In most cases you will not get this exception, because
 *  the client will automatically retry the operation call in case the server shut down the connection. However, if
 *  upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
 *  propagated to the application code.
 **/
export class CloseConnectionException extends ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::CloseConnectionException";
    }
}

/**
 *  This exception is raised by an operation call if the application closes the connection locally using
 *  {@link Connection#close}.
 *  @see Connection#close
 **/
export class ConnectionManuallyClosedException extends LocalException
{
    constructor(graceful = false, _cause = "")
    {
        super(_cause);
        this.graceful = graceful;
    }

    static get _id()
    {
        return "::Ice::ConnectionManuallyClosedException";
    }
}

/**
 *  This exception indicates that a message size is less than the minimum required size.
 **/
export class IllegalMessageSizeException extends ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::IllegalMessageSizeException";
    }
}

/**
 *  This exception indicates a problem with compressing or uncompressing data.
 **/
export class CompressionException extends ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::CompressionException";
    }
}

/**
 *  This exception is raised for errors during marshaling or unmarshaling data.
 **/
export class MarshalException extends ProtocolException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::MarshalException";
    }
}

/**
 *  This exception is raised if inconsistent data is received while unmarshaling a proxy.
 **/
export class ProxyUnmarshalException extends MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::ProxyUnmarshalException";
    }
}

/**
 *  This exception is raised if an out-of-bounds condition occurs during unmarshaling.
 **/
export class UnmarshalOutOfBoundsException extends MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::UnmarshalOutOfBoundsException";
    }
}

/**
 *  This exception is raised if no suitable value factory was found during unmarshaling of a Slice class instance.
 *  @see ValueFactory
 *  @see Communicator#getValueFactoryManager
 *  @see ValueFactoryManager#add
 *  @see ValueFactoryManager#find
 **/
export class NoValueFactoryException extends MarshalException
{
    constructor(reason, type = "", _cause = "")
    {
        super(reason, _cause);
        this.type = type;
    }

    static get _id()
    {
        return "::Ice::NoValueFactoryException";
    }
}

/**
 *  This exception is raised if the type of an unmarshaled Slice class instance does not match its expected type. This
 *  can happen if client and server are compiled with mismatched Slice definitions or if a class of the wrong type is
 *  passed as a parameter or return value using dynamic invocation. This exception can also be raised if IceStorm is
 *  used to send Slice class instances and an operation is subscribed to the wrong topic.
 **/
export class UnexpectedObjectException extends MarshalException
{
    constructor(reason, type = "", expectedType = "", _cause = "")
    {
        super(reason, _cause);
        this.type = type;
        this.expectedType = expectedType;
    }

    static get _id()
    {
        return "::Ice::UnexpectedObjectException";
    }
}

/**
 *  This exception is raised when Ice receives a request or reply message whose size exceeds the limit specified by the
 *  <code>Ice.MessageSizeMax</code> property.
 **/
export class MemoryLimitException extends MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::MemoryLimitException";
    }
}

/**
 *  This exception indicates a malformed data encapsulation.
 **/
export class EncapsulationException extends MarshalException
{
    constructor(reason, _cause = "")
    {
        super(reason, _cause);
    }

    static get _id()
    {
        return "::Ice::EncapsulationException";
    }
}

/**
 *  This exception is raised if an unsupported feature is used. The unsupported feature string contains the name of the
 *  unsupported feature.
 **/
export class FeatureNotSupportedException extends LocalException
{
    constructor(unsupportedFeature = "", _cause = "")
    {
        super(_cause);
        this.unsupportedFeature = unsupportedFeature;
    }

    static get _id()
    {
        return "::Ice::FeatureNotSupportedException";
    }
}

/**
 *  This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
 **/
export class FixedProxyException extends LocalException
{
    constructor(_cause = "")
    {
        super(_cause);
    }

    static get _id()
    {
        return "::Ice::FixedProxyException";
    }
}
