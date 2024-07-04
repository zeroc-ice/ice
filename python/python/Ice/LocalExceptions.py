# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice
from .LocalException import LocalException

class InitializationException(LocalException):
    """
        This exception is raised when a failure occurs during initialization.
    Members:
    reason --  The reason for the failure.
    """

    def __init__(self, reason=""):
        self.reason = reason

    def __str__(self):
        return "::Ice::InitializationException"
    
    __module__ = "Ice"
    __class__ = "InitializationException"


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

    def __init__(self, kindOfObject="", id=""):
        self.kindOfObject = kindOfObject
        self.id = id

    def __str__(self):
        return "::Ice::AlreadyRegisteredException"
    
    __module__ = "Ice"
    __class__ = "AlreadyRegisteredException"


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

    def __init__(self, kindOfObject="", id=""):
        self.kindOfObject = kindOfObject
        self.id = id

    def __str__(self):
        return "::Ice::NotRegisteredException"
    
    __module__ = "Ice"
    __class__ = "NotRegisteredException"


class TwowayOnlyException(LocalException):
    """
        The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
        an operation with ice_oneway, ice_batchOneway, ice_datagram, or
        ice_batchDatagram and the operation has a return value, out-parameters, or an exception specification.
    Members:
    operation --  The name of the operation that was invoked.
    """

    def __init__(self, operation=""):
        self.operation = operation

    def __str__(self):
            return "::Ice::TwowayOnlyException"
    
    __module__ = "Ice"
    __class__ = "TwowayOnlyException"


class UnknownException(LocalException):
    """
        This exception is raised if an operation call on a server raises an unknown exception. For example, for C++, this
        exception is raised if the server throws a C++ exception that is not directly or indirectly derived from
        Ice::LocalException or Ice::UserException.
    Members:
    unknown --  This field is set to the textual representation of the unknown exception if available.
    """

    def __init__(self, unknown=""):
        self.unknown = unknown

    def __str__(self):
        return "::Ice::UnknownException"
    
    __module__ = "Ice"
    __class__ = "UnknownException"


class UnknownLocalException(UnknownException):
    """
    This exception is raised if an operation call on a server raises a  local exception. Because local exceptions are
    not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
    UnknownLocalException. The only exception to this rule are all exceptions derived from
    RequestFailedException, which are transmitted by the Ice protocol even though they are declared
    local.
    """

    def __init__(self, unknown=""):
        UnknownException.__init__(self, unknown)

    def __str__(self):
        return "::Ice::UnknownLocalException"
    
    __module__ = "Ice"
    __class__ = "UnknownLocalException"


class UnknownUserException(UnknownException):
    """
    An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
    that is not declared in the exception's throws clause. Such undeclared exceptions are not transmitted
    from the server to the client by the Ice protocol, but instead the client just gets an UnknownUserException.
    This is necessary in order to not violate the contract established by an operation's signature: Only local
    exceptions and user exceptions declared in the throws clause can be raised.
    """

    def __init__(self, unknown=""):
        UnknownException.__init__(self, unknown)

    def __str__(self):
        return "::Ice::UnknownUserException"
    
    __module__ = "Ice"
    __class__ = "UnknownUserException"


class CommunicatorDestroyedException(LocalException):
    """
    This exception is raised if the Communicator has been destroyed.
    """

    def __init__(self):
        pass

    def __str__(self):
        return "::Ice::CommunicatorDestroyedException"
    
    __module__ = "Ice"
    __class__ = "CommunicatorDestroyedException"


class ObjectAdapterDeactivatedException(LocalException):
    """
        This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
    Members:
    name --  Name of the adapter.
    """

    def __init__(self, name=""):
        self.name = name

    def __str__(self):
        return "::Ice::ObjectAdapterDeactivatedException"
    
    __module__ = "Ice"
    __class__ = "ObjectAdapterDeactivatedException"


class ObjectAdapterIdInUseException(LocalException):
    """
        This exception is raised if an ObjectAdapter cannot be activated. This happens if the Locator
        detects another active ObjectAdapter with the same adapter id.
    Members:
    id --  Adapter ID.
    """

    def __init__(self, id=""):
        self.id = id

    def __str__(self):
        return "::Ice::ObjectAdapterIdInUseException"
    
    __module__ = "Ice"
    __class__ = "ObjectAdapterIdInUseException"


class NoEndpointException(LocalException):
    """
        This exception is raised if no suitable endpoint is available.
    Members:
    proxy --  The stringified proxy for which no suitable endpoint is available.
    """

    def __init__(self, proxy=""):
        self.proxy = proxy

    def __str__(self):
        return "::Ice::NoEndpointException"
    
    __module__ = "Ice"
    __class__ = "NoEndpointException"


class ParseException(LocalException):
    """
        This exception is raised if there was an error while parsing a string.
    Members:
    str --  Describes the failure and includes the string that could not be parsed.
    """

    def __init__(self, str=""):
        self.str = str

    def __str__(self):
        return "::Ice::ParseException"
    
    __module__ = "Ice"
    __class__ = "ParseException"


class IllegalIdentityException(LocalException):
    """
        This exception is raised if an identity with an empty name is encountered.
    """

    def __init__(self):
        pass

    def __str__(self):
        return "::Ice::IllegalIdentityException"
    
    __module__ = "Ice"
    __class__ = "IllegalIdentityException"


class IllegalServantException(LocalException):
    """
        This exception is raised to reject an illegal servant (typically a null servant).
    Members:
    reason --  Describes why this servant is illegal.
    """

    def __init__(self, reason=""):
        self.reason = reason

    def __str__(self):
        return "::Ice::IllegalServantException"
    
    __module__ = "Ice"
    __class__ = "IllegalServantException"


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

    def __init__(self, id=None, facet="", operation=""):
        if id is None:
            self.id = Ice.Identity()
        else:
            self.id = id
        self.facet = facet
        self.operation = operation

    def __str__(self):
        return "::Ice::RequestFailedException"
    
    __module__ = "Ice"
    __class__ = "RequestFailedException"


class ObjectNotExistException(RequestFailedException):
    """
    This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
    exist.
    """

    def __init__(self, id=None, facet="", operation=""):
        RequestFailedException.__init__(self, id, facet, operation)

    def __str__(self):
        return "::Ice::ObjectNotExistException"
    
    __module__ = "Ice"
    __class__ = "ObjectNotExistException"


class FacetNotExistException(RequestFailedException):
    """
    This exception is raised if no facet with the given name exists, but at least one facet with the given identity
    exists.
    """

    def __init__(self, id=None, facet="", operation=""):
        RequestFailedException.__init__(self, id, facet, operation)

    def __str__(self):
        return "::Ice::FacetNotExistException"
    
    __module__ = "Ice"
    __class__ = "FacetNotExistException"


class OperationNotExistException(RequestFailedException):
    """
    This exception is raised if an operation for a given object does not exist on the server. Typically this is caused
    by either the client or the server using an outdated Slice specification.
    """

    def __init__(self, id=None, facet="", operation=""):
        RequestFailedException.__init__(self, id, facet, operation)

    def __str__(self):
        return "::Ice::OperationNotExistException"
    
    __module__ = "Ice"
    __class__ = "OperationNotExistException"


class SyscallException(LocalException):
    """
        This exception is raised if a system error occurred in the server or client process. There are many possible causes
        for such a system exception. For details on the cause, SyscallException#error should be inspected.
    Members:
    error --  The error number describing the system exception. For C++ and Unix, this is equivalent to errno.
        For C++ and Windows, this is the value returned by GetLastError() or
        WSAGetLastError().
    """

    def __init__(self, error=0):
        self.error = error

    def __str__(self):
        return "::Ice::SyscallException"
    
    __module__ = "Ice"
    __class__ = "SyscallException"


class SocketException(SyscallException):
    """
    This exception indicates socket errors.
    """

    def __init__(self, error=0):
        SyscallException.__init__(self, error)

    def __str__(self):
        return "::Ice::SocketException"
    
    __module__ = "Ice"
    __class__ = "SocketException"


class FileException(SyscallException):
    """
        This exception indicates file errors.
    Members:
    path --  The path of the file responsible for the error.
    """

    def __init__(self, error=0, path=""):
        SyscallException.__init__(self, error)
        self.path = path

    def __str__(self):
        return "::Ice::FileException"
    
    __module__ = "Ice"
    __class__ = "FileException"


class ConnectFailedException(SocketException):
    """
    This exception indicates connection failures.
    """

    def __init__(self, error=0):
        SocketException.__init__(self, error)

    def __str__(self):
        return "::Ice::ConnectFailedException"
    
    __module__ = "Ice"
    __class__ = "ConnectFailedException"


class ConnectionRefusedException(ConnectFailedException):
    """
    This exception indicates a connection failure for which the server host actively refuses a connection.
    """

    def __init__(self, error=0):
        ConnectFailedException.__init__(self, error)

    def __str__(self):
        return "::Ice::ConnectionRefusedException"
    
    __module__ = "Ice"
    __class__ = "ConnectionRefusedException"


class ConnectionLostException(SocketException):
    """
    This exception indicates a lost connection.
    """

    def __init__(self, error=0):
        SocketException.__init__(self, error)

    def __str__(self):
        return "::Ice::ConnectionLostException"
    
    __module__ = "Ice"
    __class__ = "ConnectionLostException"


class DNSException(LocalException):
    """
        This exception indicates a DNS problem. For details on the cause, DNSException#error should be inspected.
    Members:
    error --  The error number describing the DNS problem. For C++ and Unix, this is equivalent to h_errno. For
        C++ and Windows, this is the value returned by WSAGetLastError().
    host --  The host name that could not be resolved.
    """

    def __init__(self, error=0, host=""):
        self.error = error
        self.host = host

    def __str__(self):
        return "::Ice::DNSException"
    
    __module__ = "Ice"
    __class__ = "DNSException"


class ConnectionIdleException(LocalException):
    """
    This exception indicates that a connection was aborted by the idle check.
    """

    def __init__(self):
        pass

    def __str__(self):
        return "::Ice::ConnectionIdleException"
    
    __module__ = "Ice"
    __class__ = "ConnectionIdleException"


class TimeoutException(LocalException):
    """
    This exception indicates a timeout condition.
    """

    def __init__(self):
        pass

    def __str__(self):
        return "::Ice::TimeoutException"
    
    __module__ = "Ice"
    __class__ = "TimeoutException"


class ConnectTimeoutException(TimeoutException):
    """
    This exception indicates a connection establishment timeout condition.
    """

    def __init__(self):
        TimeoutException.__init__(self)

    def __str__(self):
        return "::Ice::ConnectTimeoutException"
    
    __module__ = "Ice"
    __class__ = "ConnectTimeoutException"


class CloseTimeoutException(TimeoutException):
    """
    This exception indicates a connection closure timeout condition.
    """

    def __init__(self):
        TimeoutException.__init__(self)

    def __str__(self):
        return "::Ice::CloseTimeoutException"
    
    __module__ = "Ice"
    __class__ = "CloseTimeoutException"


class InvocationTimeoutException(TimeoutException):
    """
    This exception indicates that an invocation failed because it timed out.
    """

    def __init__(self):
        TimeoutException.__init__(self)

    def __str__(self):
        return "::Ice::InvocationTimeoutException"
    
    __module__ = "Ice"
    __class__ = "InvocationTimeoutException"


class InvocationCanceledException(LocalException):
    """
    This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
    """

    def __init__(self):
        pass

    def __str__(self):
        return "::Ice::InvocationCanceledException"
    
    __module__ = "Ice"
    __class__ = "InvocationCanceledException"


class ProtocolException(LocalException):
    """
        A generic exception base for all kinds of protocol error conditions.
    Members:
    reason --  The reason for the failure.
    """

    def __init__(self, reason=""):
        self.reason = reason

    def __str__(self):
        return "::Ice::ProtocolException"
    
    __module__ = "Ice"
    __class__ = "ProtocolException"


class CloseConnectionException(ProtocolException):
    """
    This exception indicates that the connection has been gracefully shut down by the server. The operation call that
    caused this exception has not been executed by the server. In most cases you will not get this exception, because
    the client will automatically retry the operation call in case the server shut down the connection. However, if
    upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
    propagated to the application code.
    """

    def __init__(self, reason=""):
        ProtocolException.__init__(self, reason)

    def __str__(self):
        return "::Ice::CloseConnectionException"
    
    __module__ = "Ice"
    __class__ = "CloseConnectionException"


class ConnectionManuallyClosedException(LocalException):
    """
        This exception is raised by an operation call if the application closes the connection locally using
        Connection#close.
    Members:
    graceful --  True if the connection was closed gracefully, false otherwise.
    """

    def __init__(self, graceful=False):
        self.graceful = graceful

    def __str__(self):
        return "::Ice::ConnectionManuallyClosedException"
    
    __module__ = "Ice"
    __class__ = "ConnectionManuallyClosedException"


class DatagramLimitException(ProtocolException):
    """
    A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
    receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
    """

    def __init__(self, reason=""):
        ProtocolException.__init__(self, reason)

    def __str__(self):
        return "::Ice::DatagramLimitException"
    
    __module__ = "Ice"
    __class__ = "DatagramLimitException"


class MarshalException(ProtocolException):
    """
    This exception is raised for errors during marshaling or unmarshaling data.
    """

    def __init__(self, reason=""):
        ProtocolException.__init__(self, reason)

    def __str__(self):
        return "::Ice::MarshalException"
    
    __module__ = "Ice"
    __class__ = "MarshalException"


class FeatureNotSupportedException(LocalException):
    """
        This exception is raised if an unsupported feature is used. The unsupported feature string contains the name of the
        unsupported feature.
    Members:
    unsupportedFeature --  The name of the unsupported feature.
    """

    def __init__(self, unsupportedFeature=""):
        self.unsupportedFeature = unsupportedFeature

    def __str__(self):
        return "::Ice::FeatureNotSupportedException"
    
    __module__ = "Ice"
    __class__ = "FeatureNotSupportedException"


class SecurityException(LocalException):
    """
        This exception indicates a failure in a security subsystem, such as the IceSSL plug-in.
    Members:
    reason --  The reason for the failure.
    """

    def __init__(self, reason=""):
        self.reason = reason

    def __str__(self):
        return "::Ice::SecurityException"
    
    __module__ = "Ice"
    __class__ = "SecurityException"


class FixedProxyException(LocalException):
    """
    This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
    """

    def __init__(self):
        pass

    def __str__(self):
        return "::Ice::FixedProxyException"
    
    __module__ = "Ice"
    __class__ = "FixedProxyException"
