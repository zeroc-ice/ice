# Copyright (c) ZeroC, Inc.

# ruff: noqa: F401  # Unused imports
# ruff: noqa: F821  # Undefined names
# ruff: noqa: E402  # Module level import not at top of file
# ruff: noqa: F403  # 'from module import *' used
# ruff: noqa: F405  # Name may be undefined from wildcard import
# ruff: noqa: I001  # Import block is unsorted or unformatted

"""
Ice module
"""

import IcePy

#
# Add some symbols to the Ice module.
#
stringVersion = IcePy.stringVersion
intVersion = IcePy.intVersion
currentProtocol = IcePy.currentProtocol
currentProtocolEncoding = IcePy.currentProtocolEncoding
currentEncoding = IcePy.currentEncoding
stringToProtocolVersion = IcePy.stringToProtocolVersion
protocolVersionToString = IcePy.protocolVersionToString
stringToEncodingVersion = IcePy.stringToEncodingVersion
encodingVersionToString = IcePy.encodingVersionToString
loadSlice = IcePy.loadSlice
AsyncInvocationContext = IcePy.AsyncInvocationContext

#
# Forward declarations.
#
from .ObjectPrxF import __Ice_ObjectPrx_t
from .ValueF import __Ice_Value_t

#
# Import local definitions that are part of the Ice module public API.
#
from .StringUtil import format_fields
from .EventLoopAdapter import EventLoopAdapter
from .Current import *
from .Future import *
from .InvocationFuture import *
from .Value import *
from .Object import Object
from .ObjectPrx import ObjectPrx, checkedCast, checkedCastAsync, uncheckedCast
from .Blobject import Blobject
from .FormatType import *
from .SlicedData import SlicedData
from .SliceInfo import SliceInfo
from .UnknownSlicedValue import *
from .ToStringMode import *
from .Exception import *
from .LocalException import *
from .UserException import *
from .Communicator import *
from .ImplicitContext import *
from .EndpointSelectionType import *
from .ObjectAdapter import *
from .CompressBatch import CompressBatch
from .ServantLocator import *
from .InitializationData import *
from .Properties import Properties
from .Logger import *
from .LocalExceptions import *
from .Proxy import *
from .Current import Current
from .Dispatch import *

#
# Import the generated code for the Ice module.
#
from .AdapterAlreadyActiveException import AdapterAlreadyActiveException
from .AdapterAlreadyActiveException import __Ice_AdapterAlreadyActiveException_t
from .AdapterNotFoundException import AdapterNotFoundException
from .AdapterNotFoundException import __Ice_AdapterNotFoundException_t
from .BTEndpointType import BTEndpointType
from .BTSEndpointType import BTSEndpointType
from .BoolSeq import __Ice_BoolSeq_t
from .ByteSeq import __Ice_ByteSeq_t
from .Context import __Ice_Context_t
from .DoubleSeq import __Ice_DoubleSeq_t
from .EncodingVersion import EncodingVersion, __Ice_EncodingVersion_t
from .FloatSeq import __Ice_FloatSeq_t
from .Identity import Identity
from .Identity import __Ice_Identity_t
from .IdentitySeq import __Ice_IdentitySeq_t
from .IntSeq import __Ice_IntSeq_t
from .InvalidReplicaGroupIdException import InvalidReplicaGroupIdException
from .InvalidReplicaGroupIdException import __Ice_InvalidReplicaGroupIdException_t
from .Locator import Locator
from .Locator import LocatorPrx
from .LocatorF import __Ice_LocatorPrx_t
from .LocatorFinder import LocatorFinder
from .LocatorFinder import LocatorFinderPrx
from .LocatorFinderF import __Ice_LocatorFinderPrx_t
from .LocatorRegistry import LocatorRegistry
from .LocatorRegistry import LocatorRegistryPrx
from .LocatorRegistryF import __Ice_LocatorRegistryPrx_t
from .LogMessage import LogMessage
from .LogMessage import __Ice_LogMessage_t
from .LogMessageSeq import __Ice_LogMessageSeq_t
from .LogMessageType import LogMessageType
from .LogMessageType import __Ice_LogMessageType_t
from .LogMessageTypeSeq import __Ice_LogMessageTypeSeq_t
from .LoggerAdmin import LoggerAdmin
from .LoggerAdmin import LoggerAdminPrx
from .LoggerAdminF import __Ice_LoggerAdminPrx_t
from .LongSeq import __Ice_LongSeq_t
from .ObjectNotFoundException import ObjectNotFoundException
from .ObjectNotFoundException import __Ice_ObjectNotFoundException_t
from .ObjectProxySeq import __Ice_ObjectProxySeq_t
from .ObjectSeq import __Ice_ObjectSeq_t
from .OperationMode import OperationMode
from .OperationMode import __Ice_OperationMode_t
from .Process import Process
from .Process import ProcessPrx
from .ProcessLogger import getProcessLogger, setProcessLogger
from .ProcessF import __Ice_ProcessPrx_t
from .PropertiesAdmin import PropertiesAdmin
from .PropertiesAdmin import PropertiesAdminPrx
from .PropertiesAdminF import __Ice_PropertiesAdminPrx_t
from .PropertyDict import __Ice_PropertyDict_t
from .ProtocolVersion import ProtocolVersion, __Ice_ProtocolVersion_t
from .RemoteLogger import RemoteLogger
from .RemoteLogger import RemoteLoggerPrx
from .RemoteLoggerAlreadyAttachedException import RemoteLoggerAlreadyAttachedException
from .RemoteLoggerAlreadyAttachedException import __Ice_RemoteLoggerAlreadyAttachedException_t
from .RemoteLoggerF import __Ice_RemoteLoggerPrx_t
from .ReplyStatus import ReplyStatus
from .ReplyStatus import __Ice_ReplyStatus_t
from .Router import Router
from .Router import RouterPrx
from .RouterF import __Ice_RouterPrx_t
from .RouterFinder import RouterFinder
from .RouterFinder import RouterFinderPrx
from .RouterFinderF import __Ice_RouterFinderPrx_t
from .SSLEndpointType import SSLEndpointType
from .ServerNotFoundException import ServerNotFoundException
from .ServerNotFoundException import __Ice_ServerNotFoundException_t
from .ShortSeq import __Ice_ShortSeq_t
from .SliceChecksumDict import __Ice_SliceChecksumDict_t
from .StringSeq import __Ice_StringSeq_t
from .TCPEndpointType import TCPEndpointType
from .UDPEndpointType import UDPEndpointType
from .URIEndpointType import URIEndpointType
from .WSEndpointType import WSEndpointType
from .WSSEndpointType import WSSEndpointType
from .iAPEndpointType import iAPEndpointType
from .iAPSEndpointType import iAPSEndpointType

from .Util import *

#
# Add EndpointInfo alias in Ice module.
#
EndpointInfo = IcePy.EndpointInfo
IPEndpointInfo = IcePy.IPEndpointInfo
TCPEndpointInfo = IcePy.TCPEndpointInfo
UDPEndpointInfo = IcePy.UDPEndpointInfo
WSEndpointInfo = IcePy.WSEndpointInfo
OpaqueEndpointInfo = IcePy.OpaqueEndpointInfo
SSLEndpointInfo = IcePy.SSLEndpointInfo

#
# Add ConnectionInfo alias in Ice module.
#
ConnectionInfo = IcePy.ConnectionInfo
IPConnectionInfo = IcePy.IPConnectionInfo
TCPConnectionInfo = IcePy.TCPConnectionInfo
UDPConnectionInfo = IcePy.UDPConnectionInfo
WSConnectionInfo = IcePy.WSConnectionInfo
SSLConnectionInfo = IcePy.SSLConnectionInfo

#
# Protocol and Encoding constants
#
Protocol_1_0 = ProtocolVersion(1, 0)
Encoding_1_0 = EncodingVersion(1, 0)
Encoding_1_1 = EncodingVersion(1, 1)

#
# Native PropertiesAdmin admin facet.
#
NativePropertiesAdmin = IcePy.NativePropertiesAdmin

# __all__ defines the public symbols of the Ice module.
# It controls what is imported when using `from Ice import *`.
# Sphinx uses this list when generating API documentation via the `automodule` directive.
__all__ = [
    "AdapterAlreadyActiveException",
    "AdapterNotFoundException",
    "AlreadyRegisteredException",
    "BTEndpointType",
    "BTSEndpointType",
    "Blobject",
    "CloseConnectionException",
    "CloseTimeoutException",
    "Communicator",
    "CommunicatorDestroyedException",
    "CompressBatch",
    "ConnectFailedException",
    "ConnectTimeoutException",
    "ConnectionAbortedException",
    "ConnectionClosedException",
    "ConnectionLostException",
    "ConnectionRefusedException",
    "Current",
    "DNSException",
    "DatagramLimitException",
    "DispatchException",
    "EncodingVersion",
    "EndpointSelectionType",
    "EventLoopAdapter",
    "Exception",
    "FacetNotExistException",
    "FeatureNotSupportedException",
    "FixedProxyException",
    "FormatType",
    "Future",
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
    "SSLEndpointType",
    "SecurityException",
    "ServantLocator",
    "ServerNotFoundException",
    "SocketException",
    "SyscallException",
    "TCPEndpointType",
    "TimeoutException",
    "ToStringMode",
    "TwowayOnlyException",
    "UDPEndpointType",
    "URIEndpointType",
    "UnknownException",
    "UnknownLocalException",
    "UnknownSlicedValue",
    "UnknownUserException",
    "UserException",
    "Value",
    "WSEndpointType",
    "WSSEndpointType",
    "__Ice_AdapterAlreadyActiveException_t",
    "__Ice_AdapterNotFoundException_t",
    "__Ice_BoolSeq_t",
    "__Ice_ByteSeq_t",
    "__Ice_Context_t",
    "__Ice_DoubleSeq_t",
    "__Ice_EncodingVersion_t",
    "__Ice_FloatSeq_t",
    "__Ice_IdentitySeq_t",
    "__Ice_Identity_t",
    "__Ice_IntSeq_t",
    "__Ice_InvalidReplicaGroupIdException_t",
    "__Ice_LocatorFinderPrx_t",
    "__Ice_LocatorPrx_t",
    "__Ice_LocatorRegistryPrx_t",
    "__Ice_LogMessageSeq_t",
    "__Ice_LogMessageTypeSeq_t",
    "__Ice_LogMessageType_t",
    "__Ice_LogMessage_t",
    "__Ice_LoggerAdminPrx_t",
    "__Ice_LongSeq_t",
    "__Ice_ObjectNotFoundException_t",
    "__Ice_ObjectProxySeq_t",
    "__Ice_ObjectSeq_t",
    "__Ice_OperationMode_t",
    "__Ice_ProcessPrx_t",
    "__Ice_PropertiesAdminPrx_t",
    "__Ice_PropertyDict_t",
    "__Ice_ProtocolVersion_t",
    "__Ice_RemoteLoggerAlreadyAttachedException_t",
    "__Ice_RemoteLoggerPrx_t",
    "__Ice_ReplyStatus_t",
    "__Ice_RouterFinderPrx_t",
    "__Ice_RouterPrx_t",
    "__Ice_ServerNotFoundException_t",
    "__Ice_ShortSeq_t",
    "__Ice_SliceChecksumDict_t",
    "__Ice_StringSeq_t",
    "createProperties",
    "format_fields",
    "getProcessLogger",
    "getSliceDir",
    "iAPEndpointType",
    "iAPSEndpointType",
    "identityToString",
    "initialize",
    "proxyIdentityCompare",
    "proxyIdentityEqual",
    "setProcessLogger",
    "stringToIdentity",
    "wrap_future",
]
