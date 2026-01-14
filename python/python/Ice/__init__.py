# Copyright (c) ZeroC, Inc.

"""
Ice module
"""

from . import asyncio

#
# Import the generated code for the Ice module.
#
from .AdapterAlreadyActiveException import AdapterAlreadyActiveException, _Ice_AdapterAlreadyActiveException_t
from .AdapterNotFoundException import AdapterNotFoundException, _Ice_AdapterNotFoundException_t
from .Blobject import Blobject
from .BoolSeq import _Ice_BoolSeq_t
from .BTEndpointType import BTEndpointType
from .BTSEndpointType import BTSEndpointType
from .Builtin import (
    BuiltinBool,
    BuiltinByte,
    BuiltinDouble,
    BuiltinFloat,
    BuiltinInt,
    BuiltinLong,
    BuiltinShort,
)
from .ByteSeq import _Ice_ByteSeq_t
from .Communicator import Communicator
from .CompressBatch import CompressBatch
from .Context import _Ice_Context_t
from .Current import Current
from .DoubleSeq import _Ice_DoubleSeq_t
from .EncodingVersion import EncodingVersion, _Ice_EncodingVersion_t
from .EndpointSelectionType import EndpointSelectionType
from .EventLoopAdapter import EventLoopAdapter
from .Exception import Exception
from .FloatSeq import _Ice_FloatSeq_t
from .FormatType import FormatType
from .Future import Future, FutureLike, wrap_future
from .iAPEndpointType import iAPEndpointType
from .iAPSEndpointType import iAPSEndpointType
from .IcePyTypes import (
    BatchRequest,
    Connection,
    ConnectionInfo,
    Endpoint,
    EndpointInfo,
    IPConnectionInfo,
    IPEndpointInfo,
    NativePropertiesAdmin,
    OpaqueEndpointInfo,
    SSLConnectionInfo,
    SSLEndpointInfo,
    TCPConnectionInfo,
    TCPEndpointInfo,
    UDPConnectionInfo,
    UDPEndpointInfo,
    WSConnectionInfo,
    WSEndpointInfo,
    intVersion,
    loadSlice,
    stringVersion,
)
from .Identity import Identity, _Ice_Identity_t
from .IdentitySeq import _Ice_IdentitySeq_t
from .ImplicitContext import ImplicitContext
from .InitializationData import InitializationData
from .IntSeq import _Ice_IntSeq_t
from .InvalidReplicaGroupIdException import InvalidReplicaGroupIdException, _Ice_InvalidReplicaGroupIdException_t
from .InvocationFuture import InvocationFuture
from .LocalException import LocalException
from .LocalExceptions import (
    AlreadyRegisteredException,
    CloseConnectionException,
    CloseTimeoutException,
    CommunicatorDestroyedException,
    ConnectFailedException,
    ConnectionAbortedException,
    ConnectionClosedException,
    ConnectionLostException,
    ConnectionRefusedException,
    ConnectTimeoutException,
    DatagramLimitException,
    DispatchException,
    DNSException,
    FacetNotExistException,
    FeatureNotSupportedException,
    FixedProxyException,
    InitializationException,
    InvocationCanceledException,
    InvocationTimeoutException,
    MarshalException,
    NoEndpointException,
    NotRegisteredException,
    ObjectAdapterDeactivatedException,
    ObjectAdapterDestroyedException,
    ObjectAdapterIdInUseException,
    ObjectNotExistException,
    OperationNotExistException,
    ParseException,
    PropertyException,
    ProtocolException,
    RequestFailedException,
    SecurityException,
    SocketException,
    SyscallException,
    TimeoutException,
    TwowayOnlyException,
    UnknownException,
    UnknownLocalException,
    UnknownUserException,
)
from .Locator import Locator, LocatorPrx
from .Locator_forward import _Ice_LocatorPrx_t
from .LocatorFinder import LocatorFinder, LocatorFinderPrx
from .LocatorFinder_forward import _Ice_LocatorFinderPrx_t
from .LocatorRegistry import LocatorRegistry, LocatorRegistryPrx
from .LocatorRegistry_forward import _Ice_LocatorRegistryPrx_t
from .Logger import Logger
from .LoggerAdmin import LoggerAdmin, LoggerAdminPrx
from .LoggerAdmin_forward import _Ice_LoggerAdminPrx_t
from .LogMessage import LogMessage, _Ice_LogMessage_t
from .LogMessageSeq import _Ice_LogMessageSeq_t
from .LogMessageType import LogMessageType, _Ice_LogMessageType_t
from .LogMessageTypeSeq import _Ice_LogMessageTypeSeq_t
from .LongSeq import _Ice_LongSeq_t
from .Object import Object
from .ObjectAdapter import ObjectAdapter
from .ObjectNotFoundException import ObjectNotFoundException, _Ice_ObjectNotFoundException_t
from .ObjectProxySeq import _Ice_ObjectProxySeq_t
from .ObjectPrx import ObjectPrx
from .ObjectSeq import _Ice_ObjectSeq_t
from .OperationMode import OperationMode, _Ice_OperationMode_t
from .Process import Process, ProcessPrx
from .Process_forward import _Ice_ProcessPrx_t
from .ProcessLogger import getProcessLogger, setProcessLogger
from .Properties import Properties
from .PropertiesAdmin import PropertiesAdmin, PropertiesAdminPrx
from .PropertiesAdmin_forward import _Ice_PropertiesAdminPrx_t
from .PropertyDict import _Ice_PropertyDict_t
from .ProtocolVersion import ProtocolVersion, _Ice_ProtocolVersion_t
from .Proxy import proxyIdentityAndFacetCompare, proxyIdentityAndFacetEqual, proxyIdentityCompare, proxyIdentityEqual
from .RemoteLogger import RemoteLogger, RemoteLoggerPrx
from .RemoteLogger_forward import _Ice_RemoteLoggerPrx_t
from .RemoteLoggerAlreadyAttachedException import (
    RemoteLoggerAlreadyAttachedException,
    _Ice_RemoteLoggerAlreadyAttachedException_t,
)
from .ReplyStatus import ReplyStatus, _Ice_ReplyStatus_t
from .Router import Router, RouterPrx
from .Router_forward import _Ice_RouterPrx_t
from .RouterFinder import RouterFinder, RouterFinderPrx
from .RouterFinder_forward import _Ice_RouterFinderPrx_t
from .ServantLocator import ServantLocator
from .ServerNotFoundException import ServerNotFoundException, _Ice_ServerNotFoundException_t
from .ShortSeq import _Ice_ShortSeq_t
from .SliceChecksumDict import _Ice_SliceChecksumDict_t
from .SlicedData import SlicedData
from .SliceInfo import SliceInfo
from .SSLEndpointType import SSLEndpointType
from .StringSeq import _Ice_StringSeq_t
from .TCPEndpointType import TCPEndpointType
from .ToStringMode import ToStringMode
from .UDPEndpointType import UDPEndpointType
from .UnknownSlicedValue import UnknownSlicedValue
from .URIEndpointType import URIEndpointType
from .UserException import UserException
from .Util import createProperties, getSliceDir, identityToString, initialize, stringToIdentity
from .Value import Value
from .WSEndpointType import WSEndpointType
from .WSSEndpointType import WSSEndpointType

#
# Protocol and Encoding constants
#
Protocol_1_0 = ProtocolVersion(1, 0)
Encoding_1_0 = EncodingVersion(1, 0)
Encoding_1_1 = EncodingVersion(1, 1)


# __all__ defines the public symbols of the Ice module.
# It controls what is imported when using `from Ice import *`.
# Sphinx uses this list when generating API documentation via the `automodule` directive.
__all__ = [
    "AdapterAlreadyActiveException",
    "AdapterNotFoundException",
    "AlreadyRegisteredException",
    "BatchRequest",
    "BTEndpointType",
    "BTSEndpointType",
    "Blobject",
    "BuiltinBool",
    "BuiltinByte",
    "BuiltinDouble",
    "BuiltinFloat",
    "BuiltinInt",
    "BuiltinLong",
    "BuiltinShort",
    "CloseConnectionException",
    "CloseTimeoutException",
    "Communicator",
    "CommunicatorDestroyedException",
    "Connection",
    "CompressBatch",
    "ConnectFailedException",
    "ConnectTimeoutException",
    "ConnectionAbortedException",
    "ConnectionClosedException",
    "ConnectionInfo",
    "ConnectionLostException",
    "ConnectionRefusedException",
    "Current",
    "DNSException",
    "DatagramLimitException",
    "DispatchException",
    "EncodingVersion",
    "Endpoint",
    "EndpointInfo",
    "EndpointSelectionType",
    "EventLoopAdapter",
    "Exception",
    "FacetNotExistException",
    "FeatureNotSupportedException",
    "FixedProxyException",
    "FormatType",
    "Future",
    "FutureLike",
    "IPConnectionInfo",
    "IPEndpointInfo",
    "Identity",
    "ImplicitContext",
    "InitializationData",
    "InitializationException",
    "InvalidReplicaGroupIdException",
    "InvocationCanceledException",
    "InvocationFuture",
    "InvocationTimeoutException",
    "LocalException",
    "Locator",
    "LocatorFinder",
    "LocatorFinderPrx",
    "LocatorPrx",
    "LocatorRegistry",
    "LocatorRegistryPrx",
    "LogMessage",
    "LogMessageType",
    "Logger",
    "LoggerAdmin",
    "LoggerAdminPrx",
    "MarshalException",
    "NativePropertiesAdmin",
    "NoEndpointException",
    "NotRegisteredException",
    "Object",
    "ObjectAdapter",
    "ObjectAdapterDeactivatedException",
    "ObjectAdapterDestroyedException",
    "ObjectAdapterIdInUseException",
    "ObjectNotExistException",
    "ObjectNotFoundException",
    "ObjectPrx",
    "OpaqueEndpointInfo",
    "OperationMode",
    "OperationNotExistException",
    "ParseException",
    "Process",
    "ProcessPrx",
    "Properties",
    "PropertiesAdmin",
    "PropertiesAdminPrx",
    "PropertyException",
    "ProtocolException",
    "ProtocolVersion",
    "RemoteLogger",
    "RemoteLoggerAlreadyAttachedException",
    "RemoteLoggerPrx",
    "ReplyStatus",
    "RequestFailedException",
    "Router",
    "RouterFinder",
    "RouterFinderPrx",
    "RouterPrx",
    "SSLConnectionInfo",
    "SSLEndpointInfo",
    "SSLEndpointType",
    "SecurityException",
    "ServantLocator",
    "ServerNotFoundException",
    "SliceInfo",
    "SlicedData",
    "SocketException",
    "SyscallException",
    "TCPConnectionInfo",
    "TCPEndpointInfo",
    "TCPEndpointType",
    "TimeoutException",
    "ToStringMode",
    "TwowayOnlyException",
    "UDPConnectionInfo",
    "UDPEndpointInfo",
    "UDPEndpointType",
    "URIEndpointType",
    "UnknownException",
    "UnknownLocalException",
    "UnknownSlicedValue",
    "UnknownUserException",
    "UserException",
    "Value",
    "WSConnectionInfo",
    "WSEndpointInfo",
    "WSEndpointType",
    "WSSEndpointType",
    "_Ice_AdapterAlreadyActiveException_t",
    "_Ice_AdapterNotFoundException_t",
    "_Ice_BoolSeq_t",
    "_Ice_ByteSeq_t",
    "_Ice_Context_t",
    "_Ice_DoubleSeq_t",
    "_Ice_EncodingVersion_t",
    "_Ice_FloatSeq_t",
    "_Ice_IdentitySeq_t",
    "_Ice_Identity_t",
    "_Ice_IntSeq_t",
    "_Ice_InvalidReplicaGroupIdException_t",
    "_Ice_LocatorFinderPrx_t",
    "_Ice_LocatorPrx_t",
    "_Ice_LocatorRegistryPrx_t",
    "_Ice_LogMessageSeq_t",
    "_Ice_LogMessageTypeSeq_t",
    "_Ice_LogMessageType_t",
    "_Ice_LogMessage_t",
    "_Ice_LoggerAdminPrx_t",
    "_Ice_LongSeq_t",
    "_Ice_ObjectNotFoundException_t",
    "_Ice_ObjectProxySeq_t",
    "_Ice_ObjectSeq_t",
    "_Ice_OperationMode_t",
    "_Ice_ProcessPrx_t",
    "_Ice_PropertiesAdminPrx_t",
    "_Ice_PropertyDict_t",
    "_Ice_ProtocolVersion_t",
    "_Ice_RemoteLoggerAlreadyAttachedException_t",
    "_Ice_RemoteLoggerPrx_t",
    "_Ice_ReplyStatus_t",
    "_Ice_RouterFinderPrx_t",
    "_Ice_RouterPrx_t",
    "_Ice_ServerNotFoundException_t",
    "_Ice_ShortSeq_t",
    "_Ice_SliceChecksumDict_t",
    "_Ice_StringSeq_t",
    "asyncio",
    "createProperties",
    "getProcessLogger",
    "getSliceDir",
    "iAPEndpointType",
    "iAPSEndpointType",
    "identityToString",
    "initialize",
    "intVersion",
    "loadSlice",
    "proxyIdentityAndFacetCompare",
    "proxyIdentityAndFacetEqual",
    "proxyIdentityCompare",
    "proxyIdentityEqual",
    "setProcessLogger",
    "stringToIdentity",
    "stringVersion",
    "wrap_future",
]
