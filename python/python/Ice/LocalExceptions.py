# Copyright (c) ZeroC, Inc.

from typing import final

from .Identity import Identity
from .LocalException import LocalException
from .ReplyStatus import ReplyStatus

__name__ = "Ice"

#
# Dispatch exceptions
#


class DispatchException(LocalException):
    """
    The exception that is raised when a dispatch failed. This is the base class for local exceptions that can be
    marshaled and transmitted "over the wire".
    You can raise this exception in the implementation of an operation, or in a middleware.
    The Ice runtime then logically re-raises this exception to the client.
    """

    def __init__(self, replyStatus: int | None, msg: str = ""):
        if replyStatus is None or replyStatus <= ReplyStatus.UserException.value or replyStatus > 255:
            raise ValueError("the reply status must fit in a byte and be greater than ReplyStatus.UserException.value")

        if msg == "":
            msg = "The dispatch failed with reply status "
            try:
                msg += ReplyStatus(replyStatus).name
            except ValueError:
                # If the replyStatus is not a valid enumerator, we just use the int value.
                # This can happen if the server uses a custom reply status.
                # We still want to provide a string representation of the reply status.
                msg += str(replyStatus)

            msg += "."
        LocalException.__init__(self, msg)
        self.__replyStatus = replyStatus

    @property
    def replyStatus(self) -> int:
        """
        Returns the reply status as an int (see :class:`ReplyStatus`).

        Returns
        -------
        int
            The reply status.
        """
        return self.__replyStatus


class RequestFailedException(DispatchException):
    """
    The base class for the 3 NotExist exceptions.
    """

    def __init__(
        self, replyStatus: int, id: Identity | None = None, facet: str = "", operation: str = "", msg: str = ""
    ):
        DispatchException.__init__(self, replyStatus, msg)
        self.__id = id if id is not None else Identity("", "")
        self.__facet = facet
        self.__operation = operation

    @property
    def id(self) -> Identity:
        """
        Returns the identity of the Ice Object to which the request was sent.

        Returns
        -------
        Identity
            The identity.
        """
        return self.__id

    @property
    def facet(self) -> str:
        """
        Returns the facet to which the request was sent.

        Returns
        -------
        str
            The facet.
        """
        return self.__facet

    @property
    def operation(self) -> str:
        """
        Returns the operation name of the request.

        Returns
        -------
        str
            The operation name.
        """
        return self.__operation


@final
class ObjectNotExistException(RequestFailedException):
    """
    The exception that is raised when a dispatch could not find a servant for the identity carried by the request.
    """

    def __init__(self, id: Identity | None = None, facet: str = "", operation: str = "", msg: str = ""):
        RequestFailedException.__init__(self, ReplyStatus.ObjectNotExist.value, id, facet, operation, msg)


@final
class FacetNotExistException(RequestFailedException):
    """
    The exception that is raised when a dispatch could not find a servant for the identity + facet carried by the
    request.
    """

    def __init__(self, id: Identity | None = None, facet: str = "", operation: str = "", msg: str = ""):
        RequestFailedException.__init__(self, ReplyStatus.FacetNotExist.value, id, facet, operation, msg)


@final
class OperationNotExistException(RequestFailedException):
    """
    The exception that is raised when a dispatch could not find the operation carried by the request on the target
    servant. This is typically due to a mismatch in the Slice definitions, such as the client using Slice
    definitions newer than the server's.
    """

    def __init__(self, id: Identity | None = None, facet: str = "", operation: str = "", msg: str = ""):
        RequestFailedException.__init__(self, ReplyStatus.OperationNotExist.value, id, facet, operation, msg)


class UnknownException(DispatchException):
    """
    The exception that is raised when a dispatch failed with an exception that is not a :class:`LocalException` or a
    :class:`UserException`.
    """

    def __init__(self, msg: str, replyStatus: int = ReplyStatus.UnknownException.value):
        DispatchException.__init__(self, replyStatus, msg)


@final
class UnknownLocalException(UnknownException):
    """
    The exception that is raised when a dispatch failed with a :class:`LocalException` that is not a
    :class:`DispatchException`.
    """

    def __init__(self, msg: str):
        UnknownException.__init__(self, msg, ReplyStatus.UnknownLocalException.value)


@final
class UnknownUserException(UnknownException):
    """
    The exception that is raised when a client receives a :class:`UserException` that was not declared in the
    operation's exception specification.
    """

    def __init__(self, msg: str):
        UnknownException.__init__(self, msg, ReplyStatus.UnknownUserException.value)


#
# Protocol exceptions
# Application code should not raise these exceptions.
#


class ProtocolException(LocalException):
    """
    The base class for exceptions related to the Ice protocol.
    """


@final
class CloseConnectionException(ProtocolException):
    """
    The exception that is raised when the connection has been gracefully shut down by the server. The request
    that returned this exception has not been executed by the server. In most cases you will not get this exception,
    because the client will automatically retry the invocation. However, if upon retry the server shuts down the
    connection again, and the retry limit has been reached, then this exception is propagated to the application code.
    """


@final
class DatagramLimitException(ProtocolException):
    """
    The exception that is raised when a datagram exceeds the configured send or receive buffer size, or exceeds the
    maximum payload size of a UDP packet (65507 bytes).
    """


@final
class MarshalException(ProtocolException):
    """
    The exception that is raised when an error occurs during marshaling or unmarshaling.
    """


#
# Timeout exceptions
# Application code should not raise these exceptions.
#


class TimeoutException(LocalException):
    """
    The exception that is raised when a timeout occurs. This is the base class for all timeout exceptions.
    """


@final
class ConnectTimeoutException(TimeoutException):
    """
    The exception that is raised when a connection establishment times out.
    """


@final
class CloseTimeoutException(TimeoutException):
    """
    The exception that is raised when a graceful connection closure times out.
    """


@final
class InvocationTimeoutException(TimeoutException):
    """
    The exception that is raised when an invocation times out.
    """


#
# Syscall exceptions
# Application code should not raise these exceptions.
#


class SyscallException(LocalException):
    """
    The exception that is raised to report the failure of a system call.
    """


@final
class DNSException(SyscallException):
    """
    The exception that is raised to report a DNS resolution failure.
    """


#
# Socket exceptions
# Application code should not raise these exceptions.
#


class SocketException(SyscallException):
    """
    The exception that is raised to report a socket error.
    """


class ConnectFailedException(SocketException):
    """
    The exception that is raised when a connection establishment fails.
    """


@final
class ConnectionLostException(SocketException):
    """
    The exception that is raised when an established connection is lost.
    """


@final
class ConnectionRefusedException(ConnectFailedException):
    """
    The exception that is raised when the server host actively refuses a connection.
    """


#
# Other leaf local exceptions in alphabetical order.
# Application code should not raise these exceptions.
#


@final
class AlreadyRegisteredException(LocalException):
    """
    The exception that is raised when you attempt to register an object more than once with the Ice runtime.
    """

    def __init__(self, kindOfObject: str, id: str, msg: str):
        LocalException.__init__(self, msg)
        self.__kindOfObject = kindOfObject
        self.__id = id

    @property
    def kindOfObject(self) -> str:
        """
        Returns the kind of object that is already registered: "servant", "facet", "object", "default servant",
        "servant locator", "plugin", "object adapter", "object adapter with router", "replica group".

        Returns
        -------
        str
            The kind.
        """
        return self.__kindOfObject

    @property
    def id(self) -> str:
        """
        Returns the ID (or name) of the object that is already registered.

        Returns
        -------
        str
            The ID (or name).
        """
        return self.__id


@final
class CommunicatorDestroyedException(LocalException):
    """
    The exception that is raised when an operation fails because the communicator has been destroyed.
    """


@final
class ConnectionAbortedException(LocalException):
    """
    The exception that is raised when an operation fails because the connection has been aborted.
    """

    def __init__(self, closedByApplication: bool, msg: str):
        LocalException.__init__(self, msg)
        self.__closedByApplication = closedByApplication

    @property
    def closedByApplication(self) -> bool:
        return self.__closedByApplication


@final
class ConnectionClosedException(LocalException):
    """
    The exception that is raised when an operation fails because the connection has been closed gracefully.
    """

    def __init__(self, closedByApplication: bool, msg: str):
        LocalException.__init__(self, msg)
        self.__closedByApplication = closedByApplication

    @property
    def closedByApplication(self) -> bool:
        return self.__closedByApplication


@final
class FeatureNotSupportedException(LocalException):
    """
    The exception that is raised when attempting to use an unsupported feature.
    """


@final
class FixedProxyException(LocalException):
    """
    The exception that is raised when attempting to change a connection-related property on a fixed proxy.
    """


@final
class InitializationException(LocalException):
    """
    The exception that is raised when communicator initialization fails.
    """


class InvocationCanceledException(LocalException):
    """
    The exception that is raised when an asynchronous invocation fails because it was canceled explicitly by the user.
    """


@final
class NoEndpointException(LocalException):
    """
    The exception that is raised when the Ice runtime cannot find a suitable endpoint to connect to.
    """


@final
class NotRegisteredException(LocalException):
    """
    The exception that is raised when attempting to find or deregister something that is not registered with Ice.
    """

    def __init__(self, kindOfObject: str, id: str, msg: str):
        LocalException.__init__(self, msg)
        self.__kindOfObject = kindOfObject
        self.__id = id

    @property
    def kindOfObject(self) -> str:
        """
        Returns the kind of object that is not registered.

        Returns
        -------
        str
            The kind.
        """
        return self.__kindOfObject

    @property
    def id(self) -> str:
        """
        Returns the ID (or name) of the object that is not registered.

        Returns
        -------
        str
            The ID (or name).
        """
        return self.__id


@final
class ObjectAdapterDeactivatedException(LocalException):
    """
    The exception that is raised when attempting to use an :class:`ObjectAdapter` that has been deactivated.
    """


@final
class ObjectAdapterDestroyedException(LocalException):
    """
    The exception that is raised when attempting to use an :class:`ObjectAdapter` that has been destroyed.
    """


@final
class ObjectAdapterIdInUseException(LocalException):
    """
    The exception that is raised when an :class:`ObjectAdapter` cannot be activated. This can happen when a
    :class:`Locator` implementation detects another active :class:`ObjectAdapter` with the same adapter ID.
    """


@final
class ParseException(LocalException):
    """
    The exception that is raised when the parsing of a string fails.
    """


@final
class SecurityException(LocalException):
    """
    The exception that is raised when a failure occurs in the security subsystem. This includes IceSSL errors.
    """


@final
class TwowayOnlyException(LocalException):
    """
    The exception that is raised when attempting to invoke an operation with ``ice_oneway``, ``ice_batchOneway``,
    ``ice_datagram``, or ``ice_batchDatagram``, and the operation has a return value, an out parameter, or an exception
    specification.
    """


@final
class PropertyException(LocalException):
    """
    The exception that is raised when a property cannot be set or retrieved.
    For example, this exception is raised when attempting to set an unknown Ice property.
    """


__all__ = [
    "DispatchException",
    "RequestFailedException",
    "ObjectNotExistException",
    "FacetNotExistException",
    "OperationNotExistException",
    "UnknownException",
    "UnknownLocalException",
    "UnknownUserException",
    "ProtocolException",
    "CloseConnectionException",
    "DatagramLimitException",
    "MarshalException",
    "TimeoutException",
    "ConnectTimeoutException",
    "CloseTimeoutException",
    "InvocationTimeoutException",
    "SyscallException",
    "DNSException",
    "SocketException",
    "ConnectFailedException",
    "ConnectionLostException",
    "ConnectionRefusedException",
    "AlreadyRegisteredException",
    "CommunicatorDestroyedException",
    "ConnectionAbortedException",
    "ConnectionClosedException",
    "FeatureNotSupportedException",
    "FixedProxyException",
    "InitializationException",
    "InvocationCanceledException",
    "NoEndpointException",
    "NotRegisteredException",
    "ObjectAdapterDeactivatedException",
    "ObjectAdapterDestroyedException",
    "ObjectAdapterIdInUseException",
    "ParseException",
    "SecurityException",
    "TwowayOnlyException",
    "PropertyException",
]
