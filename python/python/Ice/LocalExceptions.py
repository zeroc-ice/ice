# Copyright (c) ZeroC, Inc.

from .LocalException import LocalException
import Ice.ReplyStatus_ice

from typing import final

__name__ = "Ice"

#
# The 7 (8 with the RequestFailedException base class) special local exceptions that can be marshaled in an Ice
# reply message. Other local exceptions can't be marshaled. Application code can raise these exceptions.
#

class DispatchException(LocalException):
    """
    The dispatch failed. This is the base class for local exceptions that can be marshaled and transmitted "over the
    wire".
    """

    def __init__(self, replyStatus, msg=""):
        if replyStatus is None or replyStatus <= Ice.ReplyStatus.UserException.value or replyStatus > 255:
            raise ValueError("the reply status must fit in a byte and be greater than ReplyStatus.UserException.value")

        if msg == "":
            msg = "dispatch failed with reply status "
            enumerator = Ice.ReplyStatus.valueOf(replyStatus)
            msg += str(replyStatus) if enumerator is None else enumerator.name

        LocalException.__init__(self, msg)
        self.__replyStatus = replyStatus

    @property
    def replyStatus(self):
        """
        Gets the reply status of this exception.

        Returns
        -------
        int
            The reply status, as an int in the range 2..255.
        """
        return self.__replyStatus


class RequestFailedException(DispatchException):
    """
    The base exception for the 3 NotExist exceptions.
    """

    def __init__(self, replyStatus, id=None, facet="", operation="", msg=""):
        DispatchException.__init__(self, replyStatus, msg)
        self.__id = id
        self.__facet = facet
        self.__operation = operation

    @property
    def id(self):
        """
        Gets the identity of the Ice Object to which the request was sent.

        Returns
        -------
        Ice.Identity
            The identity of the Ice Object to which the request was sent.
        """
        return self.__id

    @property
    def facet(self):
        """
        Gets the facet to which the request was sent.

        Returns
        -------
        str
            The facet to which the request was sent.
        """
        return self.__facet

    @property
    def operation(self):
        """
        Gets the operation name of the request.

        Returns
        -------
        str
            The operation name of the request.
        """
        return self.__operation


@final
class ObjectNotExistException(RequestFailedException):
    """
    The dispatch could not find a servant for the identity carried by the request.
    """

    def __init__(self, id=None, facet="", operation="", msg=""):
        RequestFailedException.__init__(self, Ice.ReplyStatus.ObjectNotExist.value, id, facet, operation, msg)

@final
class FacetNotExistException(RequestFailedException):
    """
    The dispatch could not find a servant for the identity + facet carried by the request.
    """

    def __init__(self, id=None, facet="", operation="", msg=""):
        RequestFailedException.__init__(self, Ice.ReplyStatus.FacetNotExist.value, id, facet, operation, msg)


@final
class OperationNotExistException(RequestFailedException):
    """
    The dispatch could not find the operation carried by the request on the target servant. This is typically due
    to a mismatch in the Slice definitions, such as the client using Slice definitions newer than the server's.
    """

    def __init__(self, id=None, facet="", operation="", msg=""):
        RequestFailedException.__init__(self, Ice.ReplyStatus.OperationNotExist.value, id, facet, operation, msg)


class UnknownException(DispatchException):
    """
    The dispatch failed with an exception that is not a LocalException or a UserException.
    """

    def __init__(self, msg, replyStatus=Ice.ReplyStatus.UnknownException.value):
        DispatchException.__init__(self, replyStatus, msg)


@final
class UnknownLocalException(UnknownException):
    """
    The dispatch failed with LocalException that is not one of the special marshal-able local exceptions.
    """

    def __init__(self, msg):
        UnknownException.__init__(self, msg, Ice.ReplyStatus.UnknownLocalException.value)


@final
class UnknownUserException(UnknownException):
    """
    The dispatch returned a UserException that was not declared in the operation's exception specification.
    """

    def __init__(self, msg):
        UnknownException.__init__(self, msg, Ice.ReplyStatus.UnknownUserException.value)


#
# Protocol exceptions
# Application code should not raise these exceptions.
#


class ProtocolException(LocalException):
    """
    The base class for Ice protocol exceptions.
    """


@final
class CloseConnectionException(ProtocolException):
    """
    This exception indicates that the connection has been gracefully shut down by the server. The operation call that
    caused this exception has not been executed by the server. In most cases you will not get this exception, because
    the client will automatically retry the operation call in case the server shut down the connection. However, if
    upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
    propagated to the application code.
    """


@final
class DatagramLimitException(ProtocolException):
    """
    A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
    receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
    """


@final
class MarshalException(ProtocolException):
    """
    This exception reports an error during marshaling or unmarshaling.
    """


#
# Timeout exceptions
# Application code should not raise these exceptions.
#


class TimeoutException(LocalException):
    """
    This exception indicates a timeout condition.
    """


@final
class ConnectTimeoutException(TimeoutException):
    """
    This exception indicates a connection establishment timeout condition.
    """


@final
class CloseTimeoutException(TimeoutException):
    """
    This exception indicates a connection closure timeout condition.
    """


@final
class InvocationTimeoutException(TimeoutException):
    """
    This exception indicates that an invocation failed because it timed out.
    """


#
# Syscall exceptions
# Application code should not raise these exceptions.
#


class SyscallException(LocalException):
    """
    This exception is raised if a system error occurred in the server or client process.
    """


@final
class DNSException(SyscallException):
    """
    This exception indicates a DNS problem.
    """


#
# Socket exceptions
# Application code should not raise these exceptions.
#


class SocketException(SyscallException):
    """
    This exception indicates socket errors.
    """


class ConnectFailedException(SocketException):
    """
    This exception indicates connection failures.
    """


@final
class ConnectionLostException(SocketException):
    """
    This exception indicates a lost connection.
    """


@final
class ConnectionRefusedException(ConnectFailedException):
    """
    This exception indicates a connection failure for which the server host actively refuses a connection.
    """


#
# Other leaf local exceptions in alphabetical order.
# Application code should not raise these exceptions.
#


@final
class AlreadyRegisteredException(LocalException):
    """
    An attempt was made to register something more than once with the Ice run time. This exception is raised if an
    attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
    user exception factory more than once for the same ID.
    """

    def __init__(self, kindOfObject, id, msg):
        LocalException.__init__(self, msg)
        self.__kindOfObject = kindOfObject
        self.__id = id

    @property
    def kindOfObject(self):
        """
        The kind of object that could not be removed.

        This property can have one of the following values:

        - "servant"
        - "facet"
        - "object"
        - "default servant"
        - "servant locator"
        - "value factory"
        - "plugin"
        - "object adapter"
        - "object adapter with router"
        - "replica group"

        Returns
        -------
        str
            The kind of object that could not be removed.
        """
        return self.__kindOfObject

    @property
    def id(self):
        """
        The ID (or name) of the object that is already registered.

        Returns
        -------
        str
            The ID of the registered object.
        """
        return self.__id


@final
class CommunicatorDestroyedException(LocalException):
    """
    This exception is raised if the Communicator has been destroyed.
    """


@final
class ConnectionAbortedException(LocalException):
    """
    This exception indicates that a connection has been closed forcefully.
    """

    def __init__(self, closedByApplication, msg):
        LocalException.__init__(self, msg)
        self.__closedByApplication = closedByApplication

    @property
    def closedByApplication(self):
        return self.__closedByApplication


@final
class ConnectionClosedException(LocalException):
    """
    This exception indicates that a connection has been closed gracefully.
    """

    def __init__(self, closedByApplication, msg):
        LocalException.__init__(self, msg)
        self.__closedByApplication = closedByApplication

    @property
    def closedByApplication(self):
        return self.__closedByApplication


@final
class FeatureNotSupportedException(LocalException):
    """
    This exception is raised if an unsupported feature is used.
    """


@final
class FixedProxyException(LocalException):
    """
    This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
    """


@final
class InitializationException(LocalException):
    """
    This exception is raised when a failure occurs during initialization.

    Attributes
    ----------
    reason: str
        The reason for the failure.
    """


class InvocationCanceledException(LocalException):
    """
    This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
    """


@final
class NoEndpointException(LocalException):
    """
    This exception is raised if no suitable endpoint is available.
    """


@final
class NotRegisteredException(LocalException):
    """
    An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
    This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
    object adapter, object, or user exception factory that is not currently registered. It's also raised if the Ice
    locator can't find an object or object adapter when resolving an indirect proxy or when an object adapter is
    activated.
    """

    def __init__(self, kindOfObject, id, msg):
        LocalException.__init__(self, msg)
        self.__kindOfObject = kindOfObject
        self.__id = id

    @property
    def kindOfObject(self):
        """
        The kind of object that could not be removed.

        This property can have one of the following values:

        - "servant"
        - "facet"
        - "object"
        - "default servant"
        - "servant locator"
        - "value factory"
        - "plugin"
        - "object adapter"
        - "object adapter with router"
        - "replica group"

        Returns
        -------
        str
            The kind of object that could not be removed.
        """
        return self.__kindOfObject

    @property
    def id(self):
        """
        The ID (or name) of the object that could not be removed.

        Returns
        -------
        str
            The ID of the object that could not be removed.
        """
        return self.__id


@final
class ObjectAdapterDeactivatedException(LocalException):
    """
    This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
    """


@final
class ObjectAdapterDestroyedException(LocalException):
    """
    This exception is raised if an attempt is made to use a destroyed ObjectAdapter.
    """


@final
class ObjectAdapterIdInUseException(LocalException):
    """
    This exception is raised if an ObjectAdapter cannot be activated. This happens if the Locator detects another
    active ObjectAdapter with the same adapter id.
    """


@final
class ParseException(LocalException):
    """
    This exception is raised if there was an error while parsing a string.
    """


@final
class SecurityException(LocalException):
    """
    This exception indicates a failure in a security subsystem, such as the SSL transport.
    """


@final
class TwowayOnlyException(LocalException):
    """
    The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
    an operation with ice_oneway, ice_batchOneway, ice_datagram, or ice_batchDatagram and the operation has a return
    value, out-parameters, or an exception specification.
    """


@final
class PropertyException(LocalException):
    """
    This exception is raised when there is an error while getting or setting a property. For example, when
    trying to set an unknown Ice property.
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
