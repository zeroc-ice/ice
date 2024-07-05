# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from .LocalException import LocalException

class RequestFailedException(LocalException):
    """
        This exception is raised if a request failed. This exception, and all exceptions derived from
        RequestFailedException, are transmitted by the Ice protocol, even though they are declared
        local.
    Members:
    id --  The identity of the Ice Object to which the request was sent.
    facet --  The facet to which the request was sent.
    operation --  The operation name of the request.
    """

    def __init__(self, id=None, facet= "", operation="", msg=""):
        LocalException.__init__(self, msg)
        self.id = id
        self.facet = facet
        self.operation = operation

    __module__ = "Ice"

class ObjectNotExistException(RequestFailedException):
    """
    This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
    exist.
    """

    __module__ = "Ice"


class FacetNotExistException(RequestFailedException):
    """
    This exception is raised if no facet with the given name exists, but at least one facet with the given identity
    exists.
    """

    __module__ = "Ice"


class OperationNotExistException(RequestFailedException):
    """
    This exception is raised if an operation for a given object does not exist on the server. Typically this is caused
    by either the client or the server using an outdated Slice specification.
    """

    __module__ = "Ice"

class UnknownException(LocalException):
    """
        This exception is raised if an operation call on a server raises an unknown exception. For example, for C++, this
        exception is raised if the server throws a C++ exception that is not directly or indirectly derived from
        Ice::LocalException or Ice::UserException.
    """

    __module__ = "Ice"


class UnknownLocalException(UnknownException):
    """
    This exception is raised if an operation call on a server raises a  local exception. Because local exceptions are
    not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
    UnknownLocalException. The only exception to this rule are all exceptions derived from
    RequestFailedException, which are transmitted by the Ice protocol even though they are declared
    local.
    """

    __module__ = "Ice"


class UnknownUserException(UnknownException):
    """
    An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
    that is not declared in the exception's throws clause. Such undeclared exceptions are not transmitted
    from the server to the client by the Ice protocol, but instead the client just gets an UnknownUserException.
    This is necessary in order to not violate the contract established by an operation's signature: Only local
    exceptions and user exceptions declared in the throws clause can be raised.
    """

    __module__ = "Ice"


class ProtocolException(LocalException):
    """
        A generic exception base for all kinds of protocol error conditions.
    """

    __module__ = "Ice"


class CloseConnectionException(ProtocolException):
    """
    This exception indicates that the connection has been gracefully shut down by the server. The operation call that
    caused this exception has not been executed by the server. In most cases you will not get this exception, because
    the client will automatically retry the operation call in case the server shut down the connection. However, if
    upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
    propagated to the application code.
    """

    __module__ = "Ice"


class ConnectionManuallyClosedException(LocalException):
    """
        This exception is raised by an operation call if the application closes the connection locally using
        Connection#close.
    """

    __module__ = "Ice"


class DatagramLimitException(ProtocolException):
    """
    A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
    receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
    """

    __module__ = "Ice"


class MarshalException(ProtocolException):
    """
    This exception is raised for errors during marshaling or unmarshaling data.
    """

    __module__ = "Ice"


class TimeoutException(LocalException):
    """
    This exception indicates a timeout condition.
    """

    __module__ = "Ice"


class ConnectTimeoutException(TimeoutException):
    """
    This exception indicates a connection establishment timeout condition.
    """

    __module__ = "Ice"


class CloseTimeoutException(TimeoutException):
    """
    This exception indicates a connection closure timeout condition.
    """

    __module__ = "Ice"


class InvocationTimeoutException(TimeoutException):
    """
    This exception indicates that an invocation failed because it timed out.
    """

    __module__ = "Ice"


class SyscallException(LocalException):
    """
        This exception is raised if a system error occurred in the server or client process. There are many possible causes
        for such a system exception.
    """

    __module__ = "Ice"


class DNSException(SyscallException):
    """
        This exception indicates a DNS problem.
    """

    __module__ = "Ice"


class SocketException(SyscallException):
    """
    This exception indicates socket errors.
    """

    __module__ = "Ice"
    __class__ = "SocketException"


class ConnectFailedException(SocketException):
    """
    This exception indicates connection failures.
    """

    __module__ = "Ice"
    __class__ = "ConnectFailedException"


class ConnectionLostException(SocketException):
    """
    This exception indicates a lost connection.
    """

    __module__ = "Ice"


class ConnectionRefusedException(ConnectFailedException):
    """
    This exception indicates a connection failure for which the server host actively refuses a connection.
    """

    __module__ = "Ice"


class AlreadyRegisteredException(LocalException):
    """
        An attempt was made to register something more than once with the Ice run time. This exception is raised if an
        attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
        user exception factory more than once for the same ID.
    Members:
    kindOfObject --  The kind of object that could not be removed: "servant", "facet", "object", "default servant",
        "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
    id --  The ID (or name) of the object that is registered already.
    """

    def __init__(self, kindOfObject, id, msg):
        LocalException.__init__(self, msg)
        self.kindOfObject = kindOfObject
        self.id = id

    __module__ = "Ice"


class CommunicatorDestroyedException(LocalException):
    """
    This exception is raised if the Communicator has been destroyed.
    """

    __module__ = "Ice"


class ConnectionIdleException(LocalException):
    """
    This exception indicates that a connection was aborted by the idle check.
    """

    __module__ = "Ice"


class FeatureNotSupportedException(LocalException):
    """
        This exception is raised if an unsupported feature is used.
    """

    __module__ = "Ice"


class FixedProxyException(LocalException):
    """
    This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
    """

    __module__ = "Ice"


class InitializationException(LocalException):
    """
        This exception is raised when a failure occurs during initialization.
    Members:
    reason --  The reason for the failure.
    """

    __module__ = "Ice"


class InvocationCanceledException(LocalException):
    """
    This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
    """

    __module__ = "Ice"


class NoEndpointException(LocalException):
    """
        This exception is raised if no suitable endpoint is available.
    """

    __module__ = "Ice"


class NotRegisteredException(LocalException):
    """
        An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
        This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
        object adapter, object, or user exception factory that is not currently registered. It's also raised if the Ice
        locator can't find an object or object adapter when resolving an indirect proxy or when an object adapter is
        activated.
    Members:
    kindOfObject --  The kind of object that could not be removed: "servant", "facet", "object", "default servant",
        "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
    id --  The ID (or name) of the object that could not be removed.
    """

    def __init__(self, kindOfObject, id, msg):
        LocalException.__init__(self, msg)
        self.kindOfObject = kindOfObject
        self.id = id

    __module__ = "Ice"


class ObjectAdapterDeactivatedException(LocalException):
    """
        This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
    """

    def __init__(self, msg):
        LocalException.__init__(self, msg)

    __module__ = "Ice"


class ObjectAdapterIdInUseException(LocalException):
    """
        This exception is raised if an ObjectAdapter cannot be activated. This happens if the Locator
        detects another active ObjectAdapter with the same adapter id.
    """

    def __init__(self, msg):
        LocalException.__init__(self, msg)

    __module__ = "Ice"


class ParseException(LocalException):
    """
        This exception is raised if there was an error while parsing a string.
    """

    __module__ = "Ice"


class SecurityException(LocalException):
    """
        This exception indicates a failure in a security subsystem, such as the SSL transport.
    """

    __module__ = "Ice"


class TwowayOnlyException(LocalException):
    """
        The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
        an operation with ice_oneway, ice_batchOneway, ice_datagram, or
        ice_batchDatagram and the operation has a return value, out-parameters, or an exception specification.
    """

    __module__ = "Ice"
